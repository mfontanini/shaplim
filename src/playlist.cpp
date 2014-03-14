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

#include <limits>
#include "playlist.h"

playlist::playlist()
: m_current_index(), m_order(mode::default_order)
{
	std::random_device rd;
	m_generator.seed(rd());
}

void playlist::add_song(song a_song)
{
	m_songs.push_back(std::move(a_song));
	m_songs_order.push_back(m_songs.size() - 1);
	if(m_order == mode::random_order && m_songs.size() - m_current_index > 2) {
		std::uniform_int_distribution<> dis(m_current_index + 1, m_songs_order.size() - 2);
		std::swap(m_songs_order.back(), m_songs_order[dis(m_generator)]);
	}
}

bool playlist::delete_song(size_t index)
{
	if(index >= m_songs.size())
		return false;
	size_t to_delete = 0;
    for(size_t i = 0; i < m_songs_order.size(); ++i) {
        if(m_songs_order[i] == index)
            to_delete = i;
        if(m_songs_order[i] > index)
            m_songs_order[i]--;
    }
    m_songs.erase(m_songs.begin() + index);
    m_songs_order.erase(m_songs_order.begin() + to_delete);
    if(m_current_index > m_songs.size())
        m_current_index = m_songs.size();
    else if(index < m_current_index && m_current_index > 0)
        m_current_index--;
   	return true;
}

void playlist::next() 
{
	m_current_index = std::min(m_current_index + 1, m_songs.size());
}

void playlist::prev() 
{
	if(m_current_index > 0)
		m_current_index--;
}

song playlist::current() const
{
	return m_songs[m_songs_order[m_current_index]];
}

bool playlist::has_current() const
{
	return m_current_index != m_songs.size();
}

int playlist::current_index() const
{
	if(m_current_index == m_songs.size())
		return -1;
	else 
		return m_songs_order[m_current_index];
}

bool playlist::set_current_index(size_t index)
{
	if(index >= m_songs.size() || index == m_current_index)
		return false;
	m_current_index = index;
	return true;
}

void playlist::clear()
{
	m_songs.clear();
	m_songs_order.clear();
	m_current_index = 0;
}

bool playlist::songs_left() const 
{
	return m_current_index != m_songs.size();
}

const std::vector<song>& playlist::songs() const
{
	return m_songs;
}

auto playlist::playlist_mode() const -> mode
{
	return m_order;
}

void playlist::playlist_mode(mode order)
{
	m_order = order;
}

size_t playlist::song_count() const
{
	return m_songs.size();
}

bool playlist::empty() const
{
	return m_songs.empty();
}
