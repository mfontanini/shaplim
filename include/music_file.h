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

#ifndef SHAPLIM_MUSIC_FILE_H
#define SHAPLIM_MUSIC_FILE_H

#include <string>

class music_file {
public:
	music_file(std::string name);

	const std::string& name() const;
private:
	std::string m_name;
};

bool operator<(const music_file& lhs, const music_file& rhs);

#endif // SHAPLIM_MUSIC_FILE_H
