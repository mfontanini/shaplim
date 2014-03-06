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
#include <vector>
#include <fstream>
#include <string>
#include <functional>
#include <iterator>
#include <jsoncpp/json/reader.h>
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

bool read_configuration(const std::string& file_path, std::vector<std::string>& dirs) 
{
    Json::Value root;
    Json::Reader reader;
    std::ifstream input(file_path);
    std::string data{std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()};
    if(!reader.parse(data, root))
        return false;
    if(!root.isMember("shared_directories")) {
        throw std::runtime_error("Configuration file missing 'shared_directories' key");
    }
    for(const auto& dir : root["shared_directories"]) {
        dirs.push_back(dir.asString());
    }
    return true;
}

std::vector<std::string> load_shared_dirs() 
{
    std::vector<std::string> dirs;
    std::vector<std::string> config_files = {
        "shaplim.conf",
        "shaplim.conf.default"
    };
    for(const auto& config_file : config_files) {
        if(read_configuration(config_file, dirs))
            return dirs;
    }
    throw std::runtime_error("Configuration file not found");
}

int main() 
{
    try {
        std::vector<std::string> shared_dirs;
        shared_dirs = load_shared_dirs();
    	init_audio();
    	core c(shared_dirs);
    	c.run();
    }
    catch(std::runtime_error& ex) {
        std::cout << "[-] Error: " << ex.what() << std::endl;
    }
}
