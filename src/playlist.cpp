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

using locker_type = std::unique_lock<std::mutex>;

playlist::playlist()
: m_current_index(), m_order(mode::default_order)
{
	std::random_device rd;
	m_generator.seed(rd());
}

void playlist::add_song(song a_song)
{
	locker_type _(m_lock);
	m_songs.push_back(std::move(a_song));
	m_songs_order.push_back(m_songs.size() - 1);
	if(m_order == mode::random_order && m_songs.size() > 1) {
		std::uniform_int_distribution<> dis(m_current_index + 1, m_songs_order.size() - 2);
		std::swap(m_songs_order.back(), m_songs_order[dis(m_generator)]);
	}
	m_cond.notify_one();
}

bool playlist::delete_song(size_t index, const std::string& name)
{
	locker_type _(m_lock);
	if(index >= m_songs.size())
		return false;
	if(m_songs[index].path() != name)
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
	locker_type _(m_lock);
	m_current_index++;
}

void playlist::prev() 
{
	locker_type _(m_lock);
	if(m_current_index > 0)
		m_current_index--;
}

song playlist::current() const
{
	locker_type locker(m_lock);
	if(m_current_index >= m_songs.size()) {
		m_cond.wait(locker);
		if(m_current_index >= m_songs.size())
			throw std::runtime_error("No songs left");
	}
	return m_songs[m_songs_order[m_current_index]];
}

int playlist::current_index() const
{
	locker_type locker(m_lock);
	if(m_current_index >= m_songs.size())
		return -1;
	else 
		return m_songs_order[m_current_index];
}

void playlist::clear()
{
	locker_type locker(m_lock);
	m_songs.clear();
	m_songs_order.clear();
	m_current_index = 0;
	m_cond.notify_one();
}

bool playlist::songs_left() const 
{
	locker_type _(m_lock);
	return m_current_index != m_songs.size();
}

std::vector<song> playlist::songs() const
{
	locker_type _(m_lock);
	return m_songs;
}

auto playlist::playlist_mode() const -> mode
{
	locker_type _(m_lock);
	return m_order;
}

void playlist::playlist_mode(mode order)
{
	locker_type _(m_lock);
	m_order = order;
}
