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

#ifndef SHAPLIM_HTTP_H
#define SHAPLIM_HTTP_H

#include <string>
#include <map>
#include <vector>
#include <deque>
#include <mutex>
#include <tuple>
#include <condition_variable>
#include <boost/asio.hpp>

class http_request_builder {
public:
    http_request_builder(std::string url, std::string server);
    
    void add_header(std::string key, std::string value);
    void method(std::string method);
    void payload(std::string data);
    
    std::string build();
private:
    std::string m_url, m_server, m_method, m_payload;
    std::map<std::string, std::string> m_headers;
};

class http_buffer {
public:
    using chunk_type = std::vector<uint8_t>;

    http_buffer();

    template<typename InputIterator>
    void put(InputIterator start, InputIterator end) {
        put(chunk_type(start, end));
    }
    
    void put(chunk_type chunk);
    chunk_type get();
    void finish_buffer();
    bool is_finished() const;
    bool has_chunks() const;
    void max_size(size_t value);

    void reset();
private:
    // we store chunks in a deque
    std::deque<chunk_type> m_chunks;
    mutable std::mutex m_mutex;
    std::condition_variable m_empty_cond, m_full_cond;
    size_t m_current_size, m_max_size;
    bool m_finished;
};

class http_requester {
public:
    static std::tuple<std::string, std::string> split_url(const std::string& url);
    
    http_requester(boost::asio::io_service& service);
    
    void request(std::string data, const std::string& server, uint16_t port = 80);
    http_buffer& buffer() {
        return m_chunks;
    }

    size_t content_length() const;
    void stop();
private:
    enum class transfer_encoding {
        none,
        chunked,
        unknown
    };

    using streambuf_iterator = boost::asio::buffers_iterator<
        boost::asio::streambuf::const_buffers_type
    >;
    
    void process_chunked(streambuf_iterator start, streambuf_iterator end);
    void read_headers();
    void close();
    transfer_encoding find_transfer_encoding(streambuf_iterator start, streambuf_iterator end);
    size_t find_content_length(streambuf_iterator start, streambuf_iterator end);
    std::string find_location_header(streambuf_iterator start, streambuf_iterator end);
    void read_more_data();
    void read_chunk(size_t size);
    void read_content(size_t size);
    bool check_stop();

    boost::asio::ip::tcp::socket m_socket;
    boost::asio::streambuf m_buffer;
    std::string m_send_buffer;
    http_buffer m_chunks;
    size_t m_pending_chunk_length, m_content_length;
    std::atomic<bool> m_should_stop;
    std::mutex m_running_mutex;
    std::condition_variable m_condition;
    bool m_running;
};

#endif // SHAPLIM_HTTP_H
