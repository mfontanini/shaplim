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

#include "event_manager.h"

using locker_type = std::lock_guard<std::mutex>;

event::event(std::shared_ptr<Json::Value> data)
: m_data(std::move(data))
{

}

const Json::Value& event::json_data() const
{
	return *m_data;
}

std::string event::event_type() const
{
	return (*m_data)["type"].asString();
}

void event_manager::add_songs_add_event(const std::vector<std::string>& songs)
{
	std::shared_ptr<Json::Value> event_ptr = std::make_shared<Json::Value>(
		Json::objectValue
	);
	Json::Value& event = *event_ptr;
	event["type"] = "add_songs";
	event["songs"] = Json::Value(Json::arrayValue);
	for(const auto& song : songs)
		event["songs"].append(song);
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(clock_type::now(), std::move(event_ptr))
	);
}

void event_manager::add_play_song_event(int index)
{
	std::shared_ptr<Json::Value> event_ptr = std::make_shared<Json::Value>(
		Json::objectValue
	);
	Json::Value& event = *event_ptr;
	event["type"] = "play_song";
	event["index"] = index;
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(clock_type::now(), std::move(event_ptr))
	);
}

void event_manager::add_delete_songs_event(const std::vector<size_t>& indexes)
{
	std::shared_ptr<Json::Value> event_ptr = std::make_shared<Json::Value>(
		Json::objectValue
	);
	Json::Value& event = *event_ptr;
	event["type"] = "delete_songs";
	event["indexes"] = Json::arrayValue;
	auto &json_array = event["indexes"];
	for(auto index : indexes)
		json_array.append(static_cast<Json::UInt64>(index));
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(clock_type::now(), std::move(event_ptr))
	);	
}

auto event_manager::get_new_events(time_point start_point) 
	-> std::tuple<std::vector<event>, time_point>
{
	locker_type _(m_mutex);
	auto iter = m_events.lower_bound(start_point);
	auto now = clock_type::now();
	std::vector<event> output;
	while(iter != m_events.end()) {
		output.push_back(iter->second);
		++iter;
	}
	return std::make_tuple(std::move(output), now);
}

void event_manager::add_pause_event()
{
	std::shared_ptr<Json::Value> event_ptr = std::make_shared<Json::Value>(
		Json::objectValue
	);
	Json::Value& event = *event_ptr;
	event["type"] = "pause";
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(clock_type::now(), std::move(event_ptr))
	);
}

void event_manager::add_play_event()
{
	std::shared_ptr<Json::Value> event_ptr = std::make_shared<Json::Value>(
		Json::objectValue
	);
	Json::Value& event = *event_ptr;
	event["type"] = "play";
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(clock_type::now(), std::move(event_ptr))
	);
}

void event_manager::add_playlist_mode_changed_event(std::string value)
{
	std::shared_ptr<Json::Value> event_ptr = std::make_shared<Json::Value>(
		Json::objectValue
	);
	Json::Value& event = *event_ptr;
	event["type"] = "playlist_mode_changed";
	event["mode"] = std::move(value);
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(clock_type::now(), std::move(event_ptr))
	);
}

std::vector<event> event_manager::find_new_events(time_point start_point, 
	const std::string& type)
{
	locker_type _(m_mutex);
	std::vector<event> output;
	auto iter = m_events.lower_bound(start_point);
	while(iter != m_events.end()) {
		if(iter->second.event_type() == type)
			output.push_back(iter->second);
		++iter;
	}
	return output;
}
