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
#include "playback_manager.h"


playback_manager::playback_manager(types::decode_buffer_type &buffer)
: m_handle(nullptr, &Pa_CloseStream), m_buffer(buffer), m_current_rate(44100),
m_playing(false)
{
    m_params.device = Pa_GetDefaultOutputDevice();
    if (m_params.device == paNoDevice)
        throw std::runtime_error("Could not open audio device.");
    m_params.channelCount = 2;       /* stereo output */
    m_params.sampleFormat = paInt16; /* 16 bit signed integer output */
    m_params.suggestedLatency = Pa_GetDeviceInfo(m_params.device)->defaultHighOutputLatency;
    m_params.hostApiSpecificStreamInfo = NULL;

    PaStream *stream;

    if(Pa_OpenStream(&stream, NULL, &m_params, 44100, paFramesPerBufferUnspecified, paNoFlag, proxy_callback, this) != paNoError)
        throw std::runtime_error("Could not open PortAudio stream.");
   	m_handle.reset(stream);
    play();
}

void playback_manager::set_sample_rate(long rate)
{
    if(rate != m_current_rate) {
        m_playing = false;
        m_current_rate = rate;
        if(is_stream_active()) {
            Pa_StopStream(m_handle.get());
        }
        PaStream *stream;
        if(Pa_OpenStream(&stream, NULL, &m_params, rate, paFramesPerBufferUnspecified, paNoFlag, proxy_callback, this) != paNoError)
            throw std::runtime_error("Could not open PortAudio stream.");
        m_handle.reset(stream);
        play();
    }
}

bool playback_manager::play()
{
	if(!m_playing) {
		Pa_StartStream(m_handle.get());
        m_playing = true;
        return true;
    }
    return false;
}

bool playback_manager::pause()
{
	if(m_playing) {
        m_playing = false;
        return true;
    }
    return false;
}

void playback_manager::stop()
{
    Pa_StopStream(m_handle.get());
}

bool playback_manager::is_stream_active() const
{
    return Pa_IsStreamActive(m_handle.get());
}

int playback_manager::callback(void *output_buffer, unsigned long frames_per_buffer)
{
    auto buffer_ptr = static_cast<short *>(output_buffer);
    if(m_playing) {
    	m_buffer.get(
    		buffer_ptr, 
    		frames_per_buffer * 2
    	);
    }
    else {
        std::fill(
            buffer_ptr,
            buffer_ptr + frames_per_buffer * 2,
            0
        );
    }
	return paContinue;
}
