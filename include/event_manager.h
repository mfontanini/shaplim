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

#ifndef SHAPLIM_EVENT_MANAGER_H
#define SHAPLIM_EVENT_MANAGER_H

#include <chrono>
#include <map>
#include <mutex>
#include <tuple>
#include <jsoncpp/json/value.h>

class event_manager {
public:
	using clock_type = std::chrono::steady_clock;
	using time_point = clock_type::time_point;

	void add_songs_add_event(const std::vector<std::string>& songs);
	void add_play_song_event(int index);
	void add_pause_event();
	void add_play_event();
	void add_playlist_mode_changed_event(std::string value);
	std::tuple<Json::Value, time_point> get_new_events(time_point start_point);
private:
	std::map<time_point, Json::Value> m_events;
	std::mutex m_mutex;
};

#endif // SHAPLIM_EVENT_MANAGER_H
