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

#ifndef WIN32
	#include <unistd.h>
	#include <signal.h>
#endif
#include <thread>
#include <functional>
#include "types.h"
#include "mp3_decoder.h"
#include "song_stream.h"
#include "playback_manager.h"
#include "server.h"
#include "core.h"

void init_audio() 
{
	#ifndef WIN32
        int err(0);
        dup2(err, 2);
        close(2);
        if(Pa_Initialize() != paNoError)
            throw std::runtime_error("Could not initialize PortAudio.");
        dup2(2, err);
    #else
        if(Pa_Initialize() != paNoError)
            throw std::runtime_error("Could not initialize PortAudio.");
    #endif
}

int main() 
{
	init_audio();
	core c;
	c.run();
}
