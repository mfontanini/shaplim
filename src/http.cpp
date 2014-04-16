/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <boost/regex.hpp>
#include "http.h"

using boost::asio::ip::tcp;

// http_buffer

http_buffer::http_buffer()
: m_current_size(), m_max_size(10 * 1024 * 1024), m_finished(false)
{
    
}

void http_buffer::put(chunk_type chunk) 
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_current_size += chunk.size();
    m_chunks.emplace_back(std::move(chunk));
    m_empty_cond.notify_one();
    while(m_current_size >= m_max_size && !m_finished) {
        m_full_cond.wait(lock);
    }
}

auto http_buffer::get() -> chunk_type
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_chunks.empty()) {
        if(!m_finished)
            m_empty_cond.wait(lock);
    }
    chunk_type output;
    if(!m_chunks.empty()) {
        output = std::move(m_chunks.front());
        m_chunks.pop_front();
        m_current_size -= output.size();
        m_full_cond.notify_one();
    }
    return output;
}

void http_buffer::finish_buffer() 
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_finished = true;
    m_empty_cond.notify_all();
}

void http_buffer::max_size(size_t value)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_max_size = value;
}

void http_buffer::reset()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_finished = false;
}

bool http_buffer::is_finished() const
{
	std::lock_guard<std::mutex> _(m_mutex);
    return m_chunks.empty() && m_finished;
}

// http_requester

http_requester::http_requester(boost::asio::io_service& service)
: m_socket(service), m_pending_chunk_length(), m_should_stop(false),
m_running(false)
{
    
}

auto http_requester::find_transfer_encoding(streambuf_iterator start, 
    streambuf_iterator end) -> transfer_encoding
{
    static boost::regex regex("Transfer-Encoding:\\s*([^\\r]+)\\r");
    boost::match_results<streambuf_iterator> what; 
    if(regex_search(start, end, what, regex)) {
        if(std::string(what[1].first, what[1].second) == "chunked")
            return transfer_encoding::chunked;
        else
            return transfer_encoding::unknown;
    }
    return transfer_encoding::none;
}

std::string http_requester::find_location_header(streambuf_iterator start, 
    streambuf_iterator end)
{
    static boost::regex regex("Location:\\s*([^\\r]+)\\r");
    boost::match_results<streambuf_iterator> what; 
    if(regex_search(start, end, what, regex))
        return std::string(what[1].first, what[1].second);
    else
        return {};
}

size_t http_requester::find_content_length(streambuf_iterator start, 
    streambuf_iterator end)
{
    static boost::regex regex("Content-Length:\\s*([^\\r]+)\\r");
    boost::match_results<streambuf_iterator> what; 
    if(regex_search(start, end, what, regex))
        return std::stoll(std::string(what[1].first, what[1].second));
    else
        return {};
}

void http_requester::stop()
{
    m_should_stop = true;
    std::unique_lock<std::mutex> lock(m_running_mutex);
    if(m_running) {
        close();
        m_chunks.finish_buffer();
        m_condition.wait(lock);
    }
}

std::tuple<std::string, std::string> http_requester::split_url(const std::string& url)
{
    static boost::regex regex("http://([^/]+)(.*)");
    boost::match_results<std::string::const_iterator> what; 
    if(regex_search(url.begin(), url.end(), what, regex)) {
        return std::make_tuple(
            std::string(what[1].first, what[1].second),
            std::string(what[2].first, what[2].second)
        );
    }
    else
        return {};
}

void http_requester::request(std::string data, const std::string& server, 
    uint16_t port)
{
    tcp::resolver resolver(m_socket.get_io_service());
    tcp::resolver::query query(server, std::to_string(port));
    boost::system::error_code error = boost::asio::error::host_not_found;
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    while (error && endpoint_iterator != end) {
        m_socket.close();
        try {
            m_socket.connect(*endpoint_iterator++, error);
        }
        catch(std::exception&) { }
    }
    if (error) {
        throw boost::system::system_error(error);
    }
    m_send_buffer = std::move(data);
    boost::asio::async_write(
		m_socket, 
		boost::asio::buffer(m_send_buffer), 
		[&](boost::system::error_code ec, std::size_t) { 
			if(!ec)
				read_headers();
			else
				close();
		}
	);
}

size_t http_requester::content_length() const
{
    return m_content_length;
}

void http_requester::read_content(size_t size)
{
    if(check_stop()) {
        return;
    }
    // first time, maybe
    if(m_buffer.size() > 0) {
        if(m_buffer.size() > size) {
            throw std::runtime_error("Invalid response");
        }
        auto data = m_buffer.data();
        m_chunks.put(
            boost::asio::buffers_begin(data),
            boost::asio::buffers_end(data)
        );
        size -= m_buffer.size();
        m_buffer.consume(m_buffer.size());
    }
    auto this_buffer = m_buffer.prepare(2048);
    m_socket.async_read_some(
        boost::asio::buffer(this_buffer),
        [&, size](boost::system::error_code ec, std::size_t length) {
            if(!ec && length <= size) {
                m_buffer.commit(length);
                auto data = m_buffer.data();
                m_chunks.put(
                    boost::asio::buffers_begin(data),
                    boost::asio::buffers_end(data)
                );
                auto next_size = size - length;
                m_buffer.consume(m_buffer.size());
                if(next_size > 0) 
                    read_content(next_size);
                else {
                    m_chunks.finish_buffer();
                    close();
                }
            }
            else {
                if(check_stop())
                    return;
                m_chunks.finish_buffer();
                close();
            }
        }
    );
}

void http_requester::process_chunked(streambuf_iterator start, 
    streambuf_iterator end)
{
    auto end_iter = std::find(start, end, '\r');
    while(end_iter != end && std::distance(end_iter, end) >= 2) {
        auto chunk_size = std::stoll(std::string(start, end_iter), nullptr, 16);
        if(chunk_size > 0) {
            // skip \r\n
            std::advance(end_iter, 2);
            // is the chunk already here?
            if(std::distance(end_iter, end) >= chunk_size) {
                m_buffer.consume(
                    std::distance(
                        boost::asio::buffers_begin(m_buffer.data()),
                        end_iter
                    )
                );
                read_chunk(chunk_size + 2);
                auto data = m_buffer.data();
                start = boost::asio::buffers_begin(data);
                end = boost::asio::buffers_end(data);
                end_iter = std::find(
                    start, 
                    end, 
                    '\r'
                );
            }
            else {
                m_buffer.consume(
                    std::distance(
                        boost::asio::buffers_begin(m_buffer.data()),
                        end_iter
                    )
                );
                m_pending_chunk_length = chunk_size + 2;
                break;
            }
        }
        else {
            m_chunks.finish_buffer();
            return;
        }
    }
    read_more_data();
}

void http_requester::read_chunk(size_t size)
{
    auto data = m_buffer.data();
    auto start = boost::asio::buffers_begin(data);
    auto end = start + size;
    m_chunks.put(start, end - 2);
    m_pending_chunk_length = 0;
    m_buffer.consume(
        std::distance(
            start,
            end
        )
    );
}

bool http_requester::check_stop()
{
    if(m_should_stop) {
        std::lock_guard<std::mutex> _(m_running_mutex);
        if(m_running) {
            m_running = false;
            m_condition.notify_one();
        }
        return true;
    }
    return false;
}

void http_requester::read_more_data()
{
    boost::asio::async_read(
    	m_socket,
    	m_buffer,
        // Completion functor
        [&](boost::system::error_code ec, std::size_t length) -> size_t
        {
            if(check_stop())
                return 0;
            if(!ec) {
                if(m_pending_chunk_length > 0) {
                    return (m_buffer.size() >= m_pending_chunk_length) ? 0 : 1;
                }
                else {
                    auto data = m_buffer.data();
                    const auto end = boost::asio::buffers_end(data);
                    auto iter = std::find(
                        boost::asio::buffers_begin(data),
                        end,
                        '\r'
                    );
                    return (iter != end && std::distance(iter, end) >= 2) ? 0 : 1;
                }
            }
            else {
                return 0;
            }
        },
        [&](boost::system::error_code ec, std::size_t length)
        {
            if(check_stop())
                return;
            if(!ec) {
                if(m_pending_chunk_length > 0)
                    read_chunk(m_pending_chunk_length);
                auto data = m_buffer.data();
                process_chunked(
                    boost::asio::buffers_begin(data),
                    boost::asio::buffers_end(data)
                );
            }
        }
    );
}

void http_requester::close() 
{
	boost::system::error_code ignored_ec;
    m_socket.shutdown(
    	tcp::socket::shutdown_both,
        ignored_ec
    );
	m_socket.close();
}

void http_requester::read_headers()
{
    static const std::string delimiter("\r\n\r\n");
    boost::asio::async_read_until(
    	m_socket,
    	m_buffer,
    	delimiter.c_str(),
        [&](boost::system::error_code ec, std::size_t length)
        {
         	if (!ec) {
                auto data = m_buffer.data();
                auto begin = boost::asio::buffers_begin(data);
                auto end = boost::asio::buffers_end(data);
                auto iter = std::search(
                    begin,
                    boost::asio::buffers_end(data),
                    delimiter.begin(),
                    delimiter.end()
                );
                if(iter != boost::asio::buffers_end(data)) {
                    std::advance(iter, delimiter.size());
                    auto location = find_location_header(begin, iter);
                    if(!location.empty()) {
                        auto splitted = split_url(location);
                        close();
                        // empty buffer
                        m_buffer.consume(m_buffer.size());
                        http_request_builder builder(
                            std::get<1>(splitted), 
                            std::get<0>(splitted)
                        );
                        auto payload = builder.build();
                        request(payload, std::get<0>(splitted), 80);
                        return;
                    }
                    auto encoding = find_transfer_encoding(begin, iter);
                    if(encoding == transfer_encoding::chunked) {
                        process_chunked(iter, end);
                    }
                    else if(encoding == transfer_encoding::none) {
                        auto length = find_content_length(begin, end);
                        m_content_length = length;
                        // erase all headers plus last \r\n
                        m_buffer.consume(std::distance(begin, iter));
                        read_content(length);
                    }
                    else {
                        std::cout << "Not chunked!\n";
                    }
                }
                else {
                    std::cout << "Err\n";
                }
            }
            else {
                std::cout << "Error\n";
                check_stop();
                close();
            }
        }
    );
}

http_request_builder::http_request_builder(std::string url, std::string server)
: m_url(std::move(url)), m_server(std::move(server)), m_method("GET")
{
    add_header("Host", m_server);
}
    
void http_request_builder::add_header(std::string key, std::string value)
{
    m_headers.insert(std::make_pair(std::move(key), std::move(value)));
}

void http_request_builder::method(std::string method)
{
    m_method = std::move(method);
}

void http_request_builder::payload(std::string data)
{
    m_payload = std::move(data);
}

std::string http_request_builder::build()
{
    std::ostringstream oss;
    oss << m_method << ' ' << m_url << " HTTP/1.1\r\n";
    for(const auto& item : m_headers) {
        oss << item.first << ": " << item.second << "\r\n";
    }
    if(m_method == "POST") {
        oss << "Content-Length: " << m_payload.size() << "\r\n\r\n";
        oss << m_payload;
    }
    else
        oss << "\r\n\r\n";
    return oss.str();
}
