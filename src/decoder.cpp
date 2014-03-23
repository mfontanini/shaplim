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

#include "decoder.h"

void decoder::decode(song_stream stream, types::decode_buffer_type& buffer, song_type type)
{
	m_mp3_decoder.decode(std::move(stream), buffer);
}

void decoder::stop_decode()
{
	m_mp3_decoder.stop_decode();
}

float decoder::percent_so_far() 
{
	return m_mp3_decoder.percent_so_far();
}
