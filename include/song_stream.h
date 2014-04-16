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

#ifndef SHAPLIM_SONG_STREAM_H
#define SHAPLIM_SONG_STREAM_H

#include <memory>
#include <vector>

namespace boost {
namespace asio {
	class io_service;
}
}

class song_stream_impl {
public:
	using const_iterator = std::vector<unsigned char>::const_iterator;

	virtual ~song_stream_impl() {};
	virtual size_t size() = 0;
	virtual const char* buffer_ptr() = 0;
	virtual size_t available() = 0;
	virtual void advance(size_t n) = 0;
	virtual bool bytes_left() = 0;
	virtual size_t current_offset() = 0;
	virtual void seek(size_t pos) = 0;
};

class song_stream {
public:
	song_stream(std::unique_ptr<song_stream_impl> impl = nullptr);

	const char* buffer_ptr();
	size_t available();
	void advance(size_t n);
	size_t size();
	bool bytes_left();
	size_t current_offset();
	void seek(size_t pos);
private:
	std::unique_ptr<song_stream_impl> m_impl;
};

song_stream make_file_song_stream(const std::string& path);
song_stream make_youtube_song_stream(const std::string& id);

#endif // SHAPLIM_SONG_STREAM_H
