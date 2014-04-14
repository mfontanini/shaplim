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
    static boost::regex regex("\"url_encoded_fmt_stream_map\"\\s*:\\s*\"([^\"]+)\"");
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

std::string remove_parameter(std::string input, const std::string& param) 
{
    auto index = input.rfind(param);
    if(index != std::string::npos) {
        auto end = input.find('&', index + 1);
        if(end == std::string::npos)
            input.erase(input.begin() + index, input.end());
        else {
            input.erase(input.begin() + index, input.begin() + end);
        }
    }
    return input;
}

std::string remove_itag(std::string input) {
	auto output = remove_parameter(input, ",itag=");
	if(output.size() == input.size())
		output = remove_parameter(input, "&itag=");
	return output;
}

std::string normalize_url(const std::string& input) 
{
    static boost::regex regex(",([^=&,]+)=");
    return boost::regex_replace(input, regex, "&$1=", boost::match_default);
}

std::string find_video_url(std::string input) 
{
    using boost::algorithm::find_first;
    using boost::algorithm::split_regex;
    using boost::algorithm::is_any_of;
    
    input = url_decode(url_decode(input));
    boost::algorithm::replace_all(input, "\\u0026", "&");
    
    std::vector<std::string> urls;
    split_regex(urls, input, boost::regex("(?:&|,)url="));
    std::string best_url;
    for(const auto& url : urls) {
    	if(find_first(url, "quality=medium") && !find_first(url, "type=video/x-flv")) {
    		best_url = url;
    		if(find_first(url, "type=video/mp4"))
    			break;
    	}
    }
	auto this_url = remove_itag(normalize_url(best_url));
	this_url = remove_parameter(this_url, ";+codecs=");
    return this_url;
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
	const std::string& identifier, boost::asio::io_service& service)
: m_requester(service), m_current_chunk(0), m_offset(0)
{
	{
		http_request_builder request(
	        "/watch?v=" + identifier, 
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
	auto data = retrieve_song_info(payload);

	song_information info;
	info.title(std::get<0>(data));
	info.length(std::chrono::seconds(std::get<1>(data)));
	song_database::instance.set_song_info(
		"youtube://" + identifier, 
		std::move(info)
	);
    auto url = find_video_url(retrieve_urls(payload));
    auto splitted = http_requester::split_url(url);
    {
	    http_request_builder request(
	        std::get<1>(splitted), 
	        std::get<0>(splitted)
	    );
	    m_requester.request(request.build(), std::get<0>(splitted));
	}
}

void youtube_song_stream_impl::ensure_read_chunk()
{
	if(m_current_chunk == m_chunks.size() || m_iterator == current_chunk().end()) {
		auto chunk = m_requester.buffer().get();
		if(!chunk.empty()) {
			if(m_current_chunk != m_chunks.size()) {
				m_current_chunk++;
			}
			m_chunks.push_back(std::move(chunk));
			m_iterator = current_chunk().begin();
		}
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
	return std::distance(m_iterator, current_chunk().end());
}

void youtube_song_stream_impl::seek(size_t pos)
{
	std::cin.get();
	if(pos >= m_offset) {
		advance(pos - m_offset);
	}
	else {
		size_t current = 0, chunk_index = 0;
		for(const auto& chunk : m_chunks) {
			if(current + chunk.size() > pos) {
				m_offset = pos;
				m_current_chunk = chunk_index;
				m_iterator = current_chunk().begin() + (pos - current);
				break;
			}
			current += chunk.size();
			++chunk_index;
		}
	}
}

void youtube_song_stream_impl::advance(size_t n)
{
	ensure_read_chunk();
	size_t distance = std::distance(m_iterator, current_chunk().end());
	while(n > distance && m_current_chunk != m_chunks.size() && !current_chunk().empty()) {
		n -= distance;
		m_offset += distance;
		if(m_current_chunk == m_chunks.size() - 1) {
			m_iterator = current_chunk().end();
			ensure_read_chunk();
			// Didn't advance, we're done
			if(m_iterator == current_chunk().end())
				break;
		}
		else {
			++m_current_chunk;
			m_iterator = current_chunk().begin();
		}
		if(m_current_chunk != m_chunks.size())
			distance = std::distance(m_iterator, current_chunk().end());
	}
	if(!bytes_left() || current_chunk().empty())
		m_iterator = current_chunk().end();
	else {
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
	if(!m_requester.buffer().is_finished())
		return true;
	else if(m_current_chunk == m_chunks.size())
		return false;
	else
		return m_iterator != current_chunk().end();
}

size_t youtube_song_stream_impl::current_offset()
{
	return m_offset;
}

http_buffer::chunk_type& youtube_song_stream_impl::current_chunk()
{
	return m_chunks[m_current_chunk];
}