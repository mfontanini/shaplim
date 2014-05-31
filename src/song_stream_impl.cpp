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

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "song_database.h"
#include "song_stream_impl.h"

// **********************
// ** song_stream_impl **
// **********************

file_song_stream_impl::file_song_stream_impl(const std::string& path)
: m_file(path), m_base_data(m_file.data()), m_data(m_base_data)
{
	
}

size_t file_song_stream_impl::size()
{
	return m_file.size();
}

const char* file_song_stream_impl::buffer_ptr()
{
	return m_data;
}

size_t file_song_stream_impl::available()
{
	return size() - (m_data - m_file.data());
}

void file_song_stream_impl::advance(size_t n)
{
	m_data += n;
}

bool file_song_stream_impl::bytes_left()
{
	return available() != 0;
}

size_t file_song_stream_impl::current_offset()
{
    return m_data - m_base_data;
}

void file_song_stream_impl::seek(size_t pos)
{
	m_data = m_base_data + pos;
}

// **********************
// ** song_stream_impl **
// **********************

std::string retrieve_urls(const std::string& payload) {
    static boost::regex regex("url_encoded_fmt_stream_map=([^&]+)");
    boost::match_results<std::string::const_iterator> what; 
    if(regex_search(payload.begin(), payload.end(), what, regex)) {
        return std::string(what[1].first, what[1].second);
    }
    else {
        return {};
    }
}

std::string retrieve_signature(const std::string& payload) {
    static boost::regex regex("s=([^&]+)&");
    boost::match_results<std::string::const_iterator> what; 
    if(regex_search(payload.begin(), payload.end(), what, regex)) {
        return std::string(what[1].first, what[1].second);
    }
    else {
        return {};
    }
}

std::string extract_url(const std::string& payload) {
    static boost::regex regex("url=([^&]+)");
    boost::match_results<std::string::const_iterator> what; 
    if(regex_search(payload.begin(), payload.end(), what, regex)) {
        return std::string(what[1].first, what[1].second);
    }
    else {
        return {};
    }
}

std::string url_decode(const std::string& input) 
{
    std::ostringstream output;
    size_t i = 0;
    while(i < input.size()) {
        if(input[i] == '%') {
            if(i == input.size() - 2)
                throw std::runtime_error("Malformed url-encoded string");
            output << char(std::stoi(input.substr(i + 1, 2), nullptr, 16));
            i += 3;
        }
        else {
            output << input[i];
            ++i;
        }
    }
    return output.str();
}

std::string find_video_url(std::string input) 
{
    using boost::algorithm::find_first;
    using boost::algorithm::split;
    using boost::algorithm::is_any_of;
    
    input = url_decode(retrieve_urls(input));
    boost::algorithm::replace_all(input, "\\u0026", "&");
    
    std::vector<std::string> urls;
    split(urls, input, is_any_of(","));
    std::string best_url;
    for(const auto& data : urls) {
    	if(find_first(data, "quality=medium") && !find_first(data, "type=video%2Fx-flv")) {
    		best_url = extract_url(data);
            best_url = url_decode(url_decode(best_url));
            auto signature = retrieve_signature(data);
            if(!signature.empty()) {
                best_url += "&signature=" + signature;
            }
    		if(find_first(data, "type=video%2Fmp4"))
    			break;
    	}
    }
    return best_url;
}

std::tuple<std::string, size_t> retrieve_song_info(const std::string& data)
{
    static boost::regex title_regex("\"title\"\\s*:\\s*\"([^\"]+)\"");
    static boost::regex length_regex("\"length_seconds\"\\s*:\\s*([^,]+),");
    
    std::tuple<std::string, size_t> output;
    boost::match_results<std::string::const_iterator> what; 
    
    if(regex_search(data.begin(), data.end(), what, title_regex)) {
        std::get<0>(output) = std::string(what[1].first, what[1].second);
    }
    if(regex_search(data.begin(), data.end(), what, length_regex)) {
        std::get<1>(output) = std::stol(std::string(what[1].first, what[1].second));
    }
    return output;
}

youtube_song_stream_impl::youtube_song_stream_impl(
	const std::string& identifier)
: m_service_work(m_service), m_requester(m_service), m_iterator(m_chunk.end()), 
m_offset(0)
{
    m_service_thread = std::thread(
        [&]() {
            try {
                m_service.run();
            }
            catch(std::exception& ex) {
                std::cout << "Error on youtube song: " << ex.what() << std::endl;
                throw;
            }
        }
    );

	{
		http_request_builder request(
	        "/get_video_info?asv=3&el=detailpage&hl=en_US&video_id=" + identifier, 
	        "www.youtube.com"
	    );
	    m_requester.request(request.build(), "www.youtube.com");
	}
    
    auto& buffer = m_requester.buffer();
    std::string payload;
    auto chunk = buffer.get();
    while(!chunk.empty()) {
        payload.insert(payload.end(), chunk.begin(), chunk.end());
        chunk = buffer.get();
    }
    buffer.reset();
    if(payload.find("use_cipher_signature=True") != std::string::npos) {
        stop();
        throw std::runtime_error("Video signature is ciphered");
    }
	auto data = retrieve_song_info(payload);

	song_information info;
	info.title(std::get<0>(data));
	info.length(std::chrono::seconds(std::get<1>(data)));
	song_database::instance.set_song_info(
		"youtube://" + identifier, 
		std::move(info)
	);
    auto url = find_video_url(payload);
    if(url.empty()) {
        stop();
        throw std::runtime_error("Could not find youtube URL");
    }
    auto splitted = http_requester::split_url(url);
    {
	    http_request_builder request(
	        std::get<1>(splitted), 
	        std::get<0>(splitted)
	    );
	    m_requester.request(request.build(), std::get<0>(splitted));
	}
}

youtube_song_stream_impl::~youtube_song_stream_impl()
{
	stop();
}

void youtube_song_stream_impl::stop()
{
    m_service.stop();
    m_requester.stop();
    if(m_service_thread.joinable())
        m_service_thread.join();
}

void youtube_song_stream_impl::ensure_read_chunk()
{
	if(m_iterator == m_chunk.end()) {
		m_chunk = m_requester.buffer().get();
		m_iterator = m_chunk.begin();
	}
}

const char* youtube_song_stream_impl::buffer_ptr()
{
	ensure_read_chunk();
	return reinterpret_cast<const char*>(&*m_iterator);
}

size_t youtube_song_stream_impl::available()
{
	ensure_read_chunk();
	return std::distance(m_iterator, m_chunk.end());
}

void youtube_song_stream_impl::seek(size_t pos)
{
	if(pos >= m_offset) {
		advance(pos - m_offset);
	}
	else {
		throw std::runtime_error("Stream cannot be rewinded");
	}
}

void youtube_song_stream_impl::advance(size_t n)
{
	ensure_read_chunk();
	size_t distance = std::distance(m_iterator, m_chunk.end());
	while(n > distance && !m_chunk.empty()) {
		n -= distance;
		m_offset += distance;
		ensure_read_chunk();
		distance = std::distance(m_iterator, m_chunk.end());
	}
	if(m_iterator != m_chunk.end()) {
		m_iterator += n;
		m_offset += n;
	}
}

size_t youtube_song_stream_impl::size()
{
	ensure_read_chunk();
	return m_requester.content_length();
}

bool youtube_song_stream_impl::bytes_left()
{
	ensure_read_chunk();
	return !m_requester.buffer().is_finished() || m_iterator != m_chunk.end();
}

size_t youtube_song_stream_impl::current_offset()
{
	return m_offset;
}
