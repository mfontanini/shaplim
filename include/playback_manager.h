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

#ifndef SHAPLIM_PLAYBACK_MANAGER_H
#define SHAPLIM_PLAYBACK_MANAGER_H

#include <memory>
#include <atomic>
#include <portaudio.h>
#include "types.h"

class playback_manager {
public:
	playback_manager(types::decode_buffer_type &buffer);

	void set_sample_rate(long rate);
	bool play();
	bool pause();
	void stop();
	bool is_stream_active() const;
private:
	using handle_type = std::unique_ptr<PaStream, decltype(&Pa_CloseStream)>;

	playback_manager(const playback_manager&) = delete;
	playback_manager& operator=(const playback_manager&) = delete;

	int callback(void *output_buffer, unsigned long frames_per_buffer);

	static int proxy_callback(
		const void *, 
		void* buffer, 
		unsigned long fpb, 
		const PaStreamCallbackTimeInfo*, 
		PaStreamCallbackFlags, 
		void* user
	)
	{
		return (static_cast<playback_manager*>(user))->callback(buffer, fpb);
	}

	handle_type m_handle;
    PaStreamParameters m_params;
    types::decode_buffer_type &m_buffer;
    unsigned m_current_rate;
    std::atomic<bool> m_playing;
};

#endif // SHAPLIM_PLAYBACK_MANAGER_H
