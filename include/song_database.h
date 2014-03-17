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

#ifndef SHAPLIM_SONG_DATABASE_H
#define SHAPLIM_SONG_DATABASE_H

#include <map>
#include <mutex>
#include <string>
#include <chrono>

class song_information {
public:
	song_information(const std::string& file_name);

	const std::string& artist() const;
	const std::string& album() const;
	const std::string& title() const;
	const std::string& picture() const;
	const std::string& picture_mime() const;
	const std::chrono::seconds& length() const;
private:
	std::string m_artist, m_album, m_title, m_picture, m_picture_mime;
	std::chrono::seconds m_length;
};

class song_database {
public:
	const song_information& song_info(const std::string& path);
private:
	using db_type = std::map<std::string, song_information>;

	db_type m_db;
	std::mutex m_lock;
};

#endif // SHAPLIM_SONG_DATABASE_H
