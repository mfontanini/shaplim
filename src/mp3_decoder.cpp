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

#include <exception>
#include "mp3_decoder.h"

mp3_decoder::mp3_decoder()
: m_handle(nullptr, &mpg123_delete)
{
	mpg123_init();
	int err_code;
	m_handle.reset(mpg123_new(0, &err_code));
	if(!m_handle)
		throw std::runtime_error(mpg123_plain_strerror(err_code));
	mpg123_param(m_handle.get(), MPG123_ADD_FLAGS, MPG123_QUIET, 0);
}

void mp3_decoder::check_new_format()
{
	if(m_on_rate_change) {
		long rate;
		int channels, enc;
		mpg123_getformat(m_handle.get(), &rate, &channels, &enc);
		m_on_rate_change(rate);
	}
}

void mp3_decoder::decode(song_stream stream, types::decode_buffer_type &buffer)
{
	size_t size;
	const short* buf_ptr = (const short*)m_buffer.data();
	m_running = true;
	mpg123_open_feed(m_handle.get());
	while(stream.bytes_left() && m_running) {
		size_t to_read = std::min(stream.available(), m_buffer.size());
		int ret_val = mpg123_decode(
			m_handle.get(), 
			(const unsigned char*)stream.buffer_ptr(), 
			to_read, 
			(unsigned char*)m_buffer.data(), 
			m_buffer.size(), 
			&size
		);
		if(ret_val == MPG123_ERR)
			throw std::runtime_error("File decoding failed");
		stream.advance(to_read);
		if(ret_val == MPG123_NEW_FORMAT) {
			check_new_format();
		}
		buffer.put(
			buf_ptr, 
			buf_ptr + size / sizeof(short)
		);
		while(ret_val != MPG123_ERR && ret_val != MPG123_NEED_MORE) { 
			ret_val = mpg123_decode(
				m_handle.get(), 
				nullptr, 
				0, 
				(unsigned char*)m_buffer.data(), 
				m_buffer.size(), 
				&size
			);
            buffer.put(
				buf_ptr, 
				buf_ptr + size / sizeof(short)
			);
		}
		if(ret_val == MPG123_ERR)
            throw std::runtime_error("File decoding failed");
	}	
}

void mp3_decoder::stop_decode()
{
	m_running = false;
}
