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

#include "mp3_decoder.h"
#include "generic_decoder.h"
#include "decoder.h"


// TODO: create a base class for decoders.

void decoder::decode(song_stream stream, types::decode_buffer_type& buffer, song_type type)
{
	m_current_song_type = type;
	if(m_current_song_type == song_type::mp3)
		m_mp3_decoder.decode(std::move(stream), buffer);
	else
		m_generic_decoder.decode(std::move(stream), buffer);	
}

void decoder::stop_decode()
{
	if(m_current_song_type == song_type::mp3)
		m_mp3_decoder.stop_decode();
	else
		m_generic_decoder.stop_decode();
}

float decoder::percent_so_far() 
{
	if(m_current_song_type == song_type::mp3)
		return m_mp3_decoder.percent_so_far();
	else
		return m_generic_decoder.percent_so_far();
}
