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

#ifndef SHAPLIM_MP3DECODER_H
#define SHAPLIM_MP3DECODER_H

#include <array>
#include <algorithm>
#include <exception>
#include <functional>
#include <atomic>
#include <mpg123.h>
#include "types.h"
#include "song_stream.h"

template<typename T, size_t n>
class ring_buffer;

class mp3_decoder {
public:
	mp3_decoder();

	void decode(song_stream stream, types::decode_buffer_type &buffer);
	void stop_decode();
	float percent_so_far();

	template<typename Functor>
	void on_sample_rate_change(Functor callback)
	{
		m_on_rate_change = std::move(callback);
	}
private:
	using handle_type = std::unique_ptr<mpg123_handle, decltype(&mpg123_delete)>;
	using buffer_type = std::array<char, 4096>;
	static constexpr size_t chunk_size = 4096;

	void check_new_format();

	handle_type m_handle;
	buffer_type m_buffer;
	std::function<void(long long)> m_on_rate_change;
	std::atomic<off_t> m_total_size, m_start_offset, m_current_offset;
	std::atomic<bool> m_running;
};

#endif // SHAPLIM_MP3DECODER_H
