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
#include "song_stream_impl.h"

// *****************
// ** song_stream **
// *****************

song_stream::song_stream(std::unique_ptr<song_stream_impl> impl)
: m_impl(std::move(impl))
{

}

song_stream::~song_stream()
{
	if(m_impl)
		m_impl->stop();
}

const char* song_stream::buffer_ptr()
{
	return m_impl->buffer_ptr();
}

size_t song_stream::available()
{
	return m_impl->available();
}

void song_stream::advance(size_t n) 
{
	m_impl->advance(n);
}

size_t song_stream::size()
{
	return m_impl->size();
}

bool song_stream::bytes_left()
{
	return m_impl->bytes_left();
}

size_t song_stream::current_offset()
{
	return m_impl->current_offset();
}

void song_stream::seek(size_t pos)
{
	m_impl->seek(pos);
}

song_stream make_file_song_stream(const std::string& path)
{
	return song_stream(
		std::unique_ptr<song_stream_impl>(
			new file_song_stream_impl(path)
		)
	);
}

song_stream make_youtube_song_stream(const std::string& id)
{
	return song_stream(
		std::unique_ptr<song_stream_impl>(
			new youtube_song_stream_impl(id)
		)
	);
}
