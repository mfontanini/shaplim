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

#include "song_stream.h"

// **********************
// ** song_stream_impl **
// **********************

file_song_stream_impl::file_song_stream_impl(const std::string& path)
: m_file(path), m_data(m_file.data())
{
	
}

size_t file_song_stream_impl::size() const
{
	return m_file.size();
}

const char* file_song_stream_impl::buffer_ptr() const
{
	return m_data;
}

size_t file_song_stream_impl::available() const
{
	return size() - (m_data - m_file.data());
}

void file_song_stream_impl::advance(size_t n)
{
	m_data += n;
}

bool file_song_stream_impl::bytes_left() const
{
	return available() != 0;
}

// *****************
// ** song_stream **
// *****************

song_stream::song_stream(std::unique_ptr<song_stream_impl> impl)
: m_impl(std::move(impl))
{

}

const char* song_stream::buffer_ptr() const
{
	return m_impl->buffer_ptr();
}

size_t song_stream::available() const
{
	return m_impl->available();
}

void song_stream::advance(size_t n) 
{
	m_impl->advance(n);
}

size_t song_stream::size() const
{
	return m_impl->size();
}

bool song_stream::bytes_left() const
{
	return m_impl->bytes_left();
}

song_stream make_file_song_stream(const std::string& path)
{
	return song_stream(
		std::unique_ptr<song_stream_impl>(
			new file_song_stream_impl(path)
		)
	);
}
