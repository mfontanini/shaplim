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

void event_manager::add_songs_add_event(const std::vector<std::string>& songs)
{
	auto now = clock_type::now();
	Json::Value event(Json::objectValue);
	event["type"] = "add_songs";
	event["songs"] = Json::Value(Json::arrayValue);
	for(const auto& song : songs)
		event["songs"].append(song);
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(now, std::move(event))
	);
}

void event_manager::add_play_song_event(int index)
{
	auto now = clock_type::now();
	Json::Value event(Json::objectValue);
	event["type"] = "play_song";
	event["index"] = index;
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(now, std::move(event))
	);
}

auto event_manager::get_new_events(time_point start_point) 
	-> std::tuple<Json::Value, time_point>
{
	locker_type _(m_mutex);
	auto iter = m_events.lower_bound(start_point);
	auto now = clock_type::now();
	Json::Value output(Json::arrayValue);
	while(iter != m_events.end()) {
		output.append(iter->second);
		++iter;
	}
	return std::make_tuple(std::move(output), now);
}

void event_manager::add_pause_event()
{
	Json::Value event(Json::objectValue);
	event["type"] = "pause";
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(clock_type::now(), std::move(event))
	);
}

void event_manager::add_play_event()
{
	Json::Value event(Json::objectValue);
	event["type"] = "play";
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(clock_type::now(), std::move(event))
	);
}

void event_manager::add_playlist_mode_changed_event(std::string value)
{
	Json::Value event(Json::objectValue);
	event["type"] = "playlist_mode_changed";
	event["mode"] = std::move(value);
	locker_type _(m_mutex);
	m_events.insert(
		std::make_pair(clock_type::now(), std::move(event))
	);
}
