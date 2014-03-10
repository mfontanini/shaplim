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

#ifndef SHAPLIM_PLAYLIST_H
#define SHAPLIM_PLAYLIST_H

#include <vector>
#include <random>
#include <tuple>
#include "song.h"

class playlist {
public:
	enum class mode {
		default_order,
		random_order
	};

	playlist();

	void add_song(song a_song);
	bool delete_song(size_t index, const std::string& name);
	const std::vector<song>& songs() const;

	void next();
	void prev();
	song current() const;
	bool has_current() const;
	int current_index() const;
	bool songs_left() const;
	void clear();
	mode playlist_mode() const;
	void playlist_mode(mode order);
	size_t song_count() const;
	bool empty() const;
private:
	std::vector<song> m_songs;
	std::vector<unsigned int> m_songs_order;
	std::mt19937 m_generator;
	size_t m_current_index;
	mode m_order;
};

#endif // SHAPLIM_PLAYLIST_H
