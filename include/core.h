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
#include "sharing_manager.h"
#include "event_manager.h"

class core {
public:
	using shared_dirs_list = std::vector<std::string>;

	core(const shared_dirs_list& shared_dirs = shared_dirs_list());

	void run();
private:
	using command_type = std::function<Json::Value(core*, const Json::Value&)>;
	enum class playlist_actions {
		none,
		next,
		prev
	};

	void decode_loop();
	Json::Value callback(session& sess, std::string data);

	// Commands
	Json::Value add_songs(const Json::Value& params);
	Json::Value next_song(const Json::Value&);
	Json::Value previous_song(const Json::Value&);
	Json::Value playlist_mode(const Json::Value&);
	Json::Value set_playlist_mode(const Json::Value& params);
	Json::Value show_playlist(const Json::Value& params);
	Json::Value clear_playlist(const Json::Value&);
	Json::Value pause(const Json::Value&);
	Json::Value play(const Json::Value&);
	Json::Value new_events(const Json::Value& params);
	// Sharing commands
	Json::Value list_shared_dirs(const Json::Value&);
	Json::Value list_directory(const Json::Value& params);
	Json::Value add_shared_songs(const Json::Value& params);

	static std::map<std::string, command_type> m_commands;
	Json::Value json_success() const;
	Json::Value json_error(std::string error_msg) const;

	boost::asio::io_service m_io_service;
	server m_server;
	playlist m_playlist;
	types::decode_buffer_type m_buffer;
	decoder m_decoder;
	playback_manager m_playback;
	sharing_manager m_sharing_manager;
	std::thread m_decode_thread;
	playlist_actions m_next_action;
	event_manager m_event_manager;
	std::mutex m_action_lock;
};

#endif // SHAPLIM_CORE_H
