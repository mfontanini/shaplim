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

#ifndef SHAPLIM_GENERIC_DECODER_H
#define SHAPLIM_GENERIC_DECODER_H

#include <memory>
#include <atomic>
#include <functional>
#include "types.h"

extern "C" {
    #include <libavformat/avformat.h>
}

class song_stream;

class generic_decoder {
public:
	generic_decoder();

	void decode(song_stream stream, types::decode_buffer_type &buffer);
	void stop_decode();
	float percent_so_far();

	template<typename Functor>
	void on_sample_rate_change(Functor callback)
	{
		m_on_rate_change = std::move(callback);
	}
private:
	std::shared_ptr<AVFrame> m_frame;
	AVPacket m_packet;
	std::function<void(long long)> m_on_rate_change;
	std::atomic<bool> m_running;
};

#endif // SHAPLIM_GENERIC_DECODER_H
