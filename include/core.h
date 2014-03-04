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

#ifndef SHAPLIM_CORE_H
#define SHAPLIM_CORE_H

#include <jsoncpp/json/value.h>
#include <map>
#include <functional>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include "playlist.h"
#include "server.h"
#include "types.h"
#include "decoder.h"
#include "playback_manager.h"

class core {
public:
	core();

	void run();
private:
	using params_type = std::vector<std::string>;
	using command_type = std::function<Json::Value(core*, const params_type&)>;
	enum class playlist_actions {
		none,
		next,
		prev
	};

	static params_type convert(const Json::Value& value);

	void decode_loop();
	Json::Value callback(session& sess, std::string data);

	// Commands
	Json::Value add_songs(const params_type& params);
	Json::Value next_song(const params_type&);
	Json::Value previous_song(const params_type&);
	Json::Value playlist_mode(const params_type&);
	Json::Value set_playlist_mode(const params_type& params);
	Json::Value show_playlist(const params_type& params);
	Json::Value clear_playlist(const params_type&);
	Json::Value pause(const params_type&);
	Json::Value play(const params_type&);

	static std::map<std::string, command_type> m_commands;
	Json::Value json_success() const;
	Json::Value json_error(std::string error_msg) const;

	boost::asio::io_service m_io_service;
	server m_server;
	playlist m_playlist;
	types::decode_buffer_type m_buffer;
	decoder m_decoder;
	playback_manager m_playback;
	std::thread m_decode_thread;
	playlist_actions m_next_action;
	std::mutex m_action_lock;
};

#endif // SHAPLIM_CORE_H
