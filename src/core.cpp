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

#include <iostream>
#include <jsoncpp/json/reader.h>
#include <boost/algorithm/string/predicate.hpp>
#include "core.h"

using boost::algorithm::starts_with;
using locker_type = std::lock_guard<std::mutex>;

std::map<std::string, core::command_type> core::m_commands = {
	{ "add_songs", std::mem_fn(&core::add_songs) },
	{ "next_song", std::mem_fn(&core::next_song) },
	{ "previous_song", std::mem_fn(&core::previous_song) },
	{ "playlist_mode", std::mem_fn(&core::playlist_mode) },
	{ "set_playlist_mode", std::mem_fn(&core::set_playlist_mode) },
	{ "show_playlist", std::mem_fn(&core::show_playlist) },
	{ "clear_playlist", std::mem_fn(&core::clear_playlist) },
	{ "pause", std::mem_fn(&core::pause) },
	{ "play", std::mem_fn(&core::play) },
	{ "list_shared_dirs", std::mem_fn(&core::list_shared_dirs) },
	{ "list_directory", std::mem_fn(&core::list_directory) },
	{ "add_shared_songs", std::mem_fn(&core::add_shared_songs) },
	{ "new_events", std::mem_fn(&core::new_events) },
	{ "player_status", std::mem_fn(&core::player_status) },
};

class fatal_exception : public std::exception {
public:
	const char* what() const noexcept {
		return "Fatal error";
	}
};

core::core(const shared_dirs_list& shared_dirs)
: m_server(m_io_service, 1337), m_playback(m_buffer), 
m_sharing_manager(shared_dirs),
m_next_action(playlist_actions::none)
{
	m_decoder.on_sample_rate_change(
		std::bind(&playback_manager::set_sample_rate, &m_playback, std::placeholders::_1)
	);
	m_server.on_data_available(
		std::bind(
			&core::callback, 
			this, 
			std::placeholders::_1, 
			std::placeholders::_2
		)
	);
}

void core::run()
{
	m_decode_thread = std::thread(&core::decode_loop, this);
	m_io_service.run();
}

void core::decode_loop()
{
	while(true) {
		song_stream stream;
		try {
			song song_to_play;
			int current_index;

			{
				std::unique_lock<std::mutex> lock(m_playlist_mutex);
				execute_next_action();
				while(!m_playlist.has_current()) {
					m_playlist_cond.wait(lock);
					execute_next_action();
				}
				song_to_play = m_playlist.current();
				current_index = m_playlist.current_index();
				m_next_action = playlist_actions::next;
			}
			std::cout << song_to_play.path() << std::endl;
			m_event_manager.add_play_song_event(current_index);
			if(song_to_play.schema() == song::schema_type::file)
				stream = make_file_song_stream(song_to_play.path());
			else {
				std::cout << "Unknown schema for " << song_to_play.path() << std::endl;
			}
			m_decoder.decode(std::move(stream), m_buffer, decoder::song_type::mp3);
		}
		catch(std::exception& ex) {
			std::cout << "Error: " << ex.what() << std::endl;
		}
	}
}

void core::execute_next_action()
{
	switch(m_next_action) {
		case playlist_actions::next:
			m_playlist.next();
			break;
		case playlist_actions::prev:
			m_playlist.prev();
			break;
		default:
			break;
	}
	m_next_action = playlist_actions::none;
}

Json::Value core::callback(session& sess, std::string data)
{
	Json::Value result(Json::objectValue);
	try {
		Json::Value root;
		Json::Reader reader;
		if(!reader.parse(data, root) || !root.isMember("type"))
			throw fatal_exception();
		else {
			std::string type = root["type"].asString();
			auto iter = m_commands.find(type);
			if(iter == m_commands.end())
				throw std::runtime_error("Invalid command type");
			result = iter->second(this, root["params"]);
		}
	}
	catch(fatal_exception& ex) {
		std::cout << "Fatal\n";
		throw;
	}
	catch(std::exception& ex) {
		result["result"] = false;
		result["message"] = ex.what();
	}
	return result;
}

Json::Value core::json_success() const
{
	Json::Value output(Json::objectValue);
	output["result"] = true;
	return output;
}

Json::Value core::json_error(std::string error_msg) const
{
	Json::Value output(Json::objectValue);
	output["result"] = false;
	output["message"] = std::move(error_msg);
	return output;
}

// Commands

Json::Value core::add_songs(const Json::Value& params) 
{
	std::vector<std::string> songs;
	{
		locker_type _(m_playlist_mutex);
		for(const auto& song : params) {
			auto name = song.asString();
			m_playlist.add_song(name);
			songs.push_back(std::move(name));
		}
		m_playlist_cond.notify_one();
	}
	m_event_manager.add_songs_add_event(songs);
	return json_success();
}

Json::Value core::next_song(const Json::Value&) 
{
	{
		locker_type _(m_playlist_mutex);
		m_next_action = playlist_actions::next;
		m_decoder.stop_decode();
	}
	return json_success();
}

Json::Value core::previous_song(const Json::Value&)
{
	{
		locker_type _(m_playlist_mutex);
		m_next_action = playlist_actions::prev;
		m_decoder.stop_decode();
		m_playlist_cond.notify_one();
	}
	return json_success();
}

Json::Value core::show_playlist(const Json::Value& params)
{
	auto now = event_manager::clock_type::now();
	Json::Value output(Json::objectValue);
	{
		// Lock to retrieve songs
		locker_type _(m_playlist_mutex);
		const auto& songs = m_playlist.songs();
		output["songs"] = Json::Value(Json::arrayValue);
		for(const auto& item : songs) {
			const auto& path = item.path();
			auto index = path.rfind('/');
			output["songs"].append(path.substr(index + 1));
		}
		output["current"] = static_cast<Json::Int>(m_playlist.current_index());
	}
	output["result"] = true;
	output["timestamp"] = static_cast<Json::UInt64>(
		now.time_since_epoch().count()
	);
	return output;
}

Json::Value core::playlist_mode(const Json::Value&)
{
	playlist::mode mode;
	{
		locker_type _(m_playlist_mutex);
		mode = m_playlist.playlist_mode();
	}
	Json::Value output(Json::objectValue);
	output["result"] = true;
	if(mode == playlist::mode::random_order)
		output["mode"] = "shuffle";
	else
		output["mode"] = "default";
	return output;
}

Json::Value core::set_playlist_mode(const Json::Value& params)
{
	auto param = params.asString();
	locker_type _(m_playlist_mutex);
	if(params == "shuffle")
		m_playlist.playlist_mode(playlist::mode::random_order);
	else if(params != "default")
		m_playlist.playlist_mode(playlist::mode::default_order);
	else
		return json_error("Valid modes are 'shuffle' and 'default'");
	return json_success();
}

Json::Value core::clear_playlist(const Json::Value&)
{
	{
		locker_type _(m_playlist_mutex);
		m_next_action = playlist_actions::none;
		m_decoder.stop_decode();
		m_playlist.clear();
	}
	return json_success();
}

Json::Value core::pause(const Json::Value&)
{
	if(m_playback.pause())
		m_event_manager.add_pause_event();
	return json_success();
}

Json::Value core::play(const Json::Value&)
{
	if(m_playback.play())
		m_event_manager.add_play_event();
	return json_success();
}

Json::Value core::player_status(const Json::Value&)
{
	Json::Value output(Json::objectValue);
	output["result"] = true;
	output["status"] = m_playback.is_stream_active() ? "playing" : "paused";
	return output;
}

Json::Value core::list_shared_dirs(const Json::Value&)
{
	auto dirs = m_sharing_manager.shared_directories();
	Json::Value output(Json::objectValue);
	output["directories"] = Json::Value(Json::arrayValue);
	for(const auto& dir : dirs)
		output["directories"].append(dir);
	output["result"] = true;
	return output;
}

Json::Value core::list_directory(const Json::Value& params)
{
	auto param = params.asString();
	const auto& root_dir = m_sharing_manager.find_directory(param);
	Json::Value output(Json::objectValue);
	output["directories"] = Json::Value(Json::arrayValue);
	output["files"] = Json::Value(Json::arrayValue);
	for(const auto& dir : root_dir.directories()) {
		output["directories"].append(dir.name().string());
	}
	for(const auto& file : root_dir.files())
		output["files"].append(file.name());
	output["result"] = true;
	return output;
}

Json::Value core::add_shared_songs(const Json::Value& params)
{
	if(!params.isMember("base_path") || !params.isMember("songs"))
		return json_error("Expected 'base_path' and 'songs' keys");
	auto base_path = params["base_path"].asString();
	const auto& root_dir = m_sharing_manager.find_directory(base_path);
	std::vector<std::string> songs;
	{
		locker_type _(m_playlist_mutex);
		for(const auto& key : params["songs"]) {
			auto song_path = root_dir.path_for_file(key.asString());
			m_playlist.add_song(song_path);
			songs.push_back(std::move(song_path));
		}
		m_playlist_cond.notify_one();
	}
	m_event_manager.add_songs_add_event(songs);
	return json_success();
}

Json::Value core::new_events(const Json::Value& params)
{
	const auto param = params.asUInt64();
	const auto duration = event_manager::time_point::duration(param);
	auto events_tuple = m_event_manager.get_new_events(
		event_manager::time_point(duration)
	);
	Json::Value output(Json::objectValue);
	output["events"] = Json::Value(Json::arrayValue);
	for(const auto& event : std::get<0>(events_tuple))
		output["events"].append(event);
	output["result"] = true;
	output["timestamp"] = static_cast<Json::UInt64>(
		std::get<1>(events_tuple).time_since_epoch().count()
	);
	return output;
}
