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

#ifndef SHAPLIM_SONG_STREAM_IMPL_H
#define SHAPLIM_SONG_STREAM_IMPL_H

#include <boost/iostreams/device/mapped_file.hpp>
#include <thread>
#include "song_stream.h"
#include "http.h"

class file_song_stream_impl : public song_stream_impl {
public:
	file_song_stream_impl(const std::string& path);
	const char* buffer_ptr();
	size_t available();
	void advance(size_t n);
	size_t size();
	void seek(size_t pos);
	bool bytes_left();
	size_t current_offset();
private:
	boost::iostreams::mapped_file_source m_file;
	const char* m_base_data, *m_data;
};

class youtube_song_stream_impl : public song_stream_impl {
public:
	youtube_song_stream_impl(const std::string& identifier);
	~youtube_song_stream_impl();

	const char* buffer_ptr();
	size_t available();
	void advance(size_t n);
	void seek(size_t pos);
	size_t size();
	bool bytes_left();
	size_t current_offset();
private:
	void ensure_read_chunk();

	boost::asio::io_service m_service;
	boost::asio::io_service::work m_service_work;
	http_requester m_requester;
	http_buffer::chunk_type m_chunk;
	http_buffer::chunk_type::iterator m_iterator;
	std::thread m_service_thread;
	size_t m_offset;
};

#endif // SHAPLIM_SONG_STREAM_IMPL_H
