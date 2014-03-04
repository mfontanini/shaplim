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
};

class fatal_exception : public std::exception {
public:
	const char* what() const noexcept {
		return "Fatal error";
	}
};

auto core::convert(const Json::Value& value) -> params_type
{
	params_type output;
	for(auto&& item : value) {
		output.push_back(item.asString());
	}
	return output;
}

core::core()
: m_server(m_io_service, 1337), m_playback(m_buffer), 
m_next_action(playlist_actions::next)
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
			song song_to_play = m_playlist.current();
			std::cout << song_to_play.path() << std::endl;
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
		locker_type _(m_action_lock);
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
		m_next_action = playlist_actions::next;
	}
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
			params_type params;
			if(root.isMember("params"))
				params = convert(root["params"]);
			auto iter = m_commands.find(type);
			if(iter == m_commands.end())
				throw std::runtime_error("Invalid command type");
			result = iter->second(this, params);
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

Json::Value core::add_songs(const params_type& params) 
{
	for(const auto& song : params) {
		m_playlist.add_song(song);
	}
	return json_success();
}

Json::Value core::next_song(const params_type&) 
{
	{
		locker_type _(m_action_lock);
		m_next_action = playlist_actions::next;
		m_decoder.stop_decode();
	}
	return json_success();
}

Json::Value core::previous_song(const params_type&)
{
	{
		locker_type _(m_action_lock);
		m_next_action = playlist_actions::prev;
		m_decoder.stop_decode();
	}
	return json_success();
}

Json::Value core::show_playlist(const params_type& params)
{
	auto songs = m_playlist.songs();
	Json::Value output(Json::objectValue);
	output["songs"] = Json::Value(Json::arrayValue);
	for(const auto& item : songs) {
		const auto& path = item.path();
		auto index = path.rfind('/');
		output["songs"].append(path.substr(index + 1));
	}
	output["current"] = static_cast<Json::Int>(m_playlist.current_index());
	output["result"] = true;
	return output;
}

Json::Value core::playlist_mode(const params_type&)
{
	auto mode = m_playlist.playlist_mode();
	Json::Value output(Json::objectValue);
	output["result"] = true;
	if(mode == playlist::mode::random_order)
		output["mode"] = "shuffle";
	else
		output["mode"] = "default";
	return output;
}

Json::Value core::set_playlist_mode(const params_type& params)
{
	if(params.size() != 1)
		return json_error("Expected one parameter");
	if(params.front() == "shuffle")
		m_playlist.playlist_mode(playlist::mode::random_order);
	else if(params.front() != "default")
		m_playlist.playlist_mode(playlist::mode::default_order);
	else
		return json_error("Valid modes are \"shuffle\" and \"default\"");
	return json_success();
}

Json::Value core::clear_playlist(const params_type&)
{
	{
		locker_type _(m_action_lock);
		m_next_action = playlist_actions::none;
		m_decoder.stop_decode();
	}
	m_playlist.clear();
	return json_success();
}

Json::Value core::pause(const params_type&)
{
	m_playback.pause();
	return json_success();
}

Json::Value core::play(const params_type&)
{
	m_playback.play();
	return json_success();
}
