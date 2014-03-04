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
#include <boost/iostreams/device/mapped_file.hpp>

class song_stream_impl {
public:
	using const_iterator = std::vector<unsigned char>::const_iterator;

	virtual ~song_stream_impl() {};
	virtual size_t size() const = 0;
	virtual const char* buffer_ptr() const = 0;
	virtual size_t available() const = 0;
	virtual void advance(size_t n) = 0;
	virtual bool bytes_left() const = 0;
};

class file_song_stream_impl : public song_stream_impl {
public:
	file_song_stream_impl(const std::string& path);
	const char* buffer_ptr() const;
	size_t available() const;
	void advance(size_t n);
	size_t size() const;
	bool bytes_left() const;
private:
	boost::iostreams::mapped_file_source m_file;
	const char* m_data;
};

class song_stream {
public:
	song_stream(std::unique_ptr<song_stream_impl> impl = nullptr);

	const char* buffer_ptr() const;
	size_t available() const;
	void advance(size_t n);
	size_t size() const;
	bool bytes_left() const;
private:
	std::unique_ptr<song_stream_impl> m_impl;
};

song_stream make_file_song_stream(const std::string& path);

#endif // SHAPLIM_SONG_STREAM_H
