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

#ifndef SHAPLIM_DECODER_H
#define SHAPLIM_DECODER_H

#include "mp3_decoder.h"
#include "song_stream.h"
#include "types.h"

class decoder {
public:
	enum class song_type {
		mp3
	};

	template<typename Functor>
	void on_sample_rate_change(Functor callback) 
	{
		m_mp3_decoder.on_sample_rate_change(callback);
	}

	void decode(song_stream stream, types::decode_buffer_type& buffer, song_type type);
	void stop_decode();
	float percent_so_far();
private:
	mp3_decoder m_mp3_decoder;
};

#endif // SHAPLIM_DECODER_H
