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

#include "song.h"

song::song()
{

}

song::song(std::string path, schema_type song_schema)
: m_path(std::move(path)), m_schema(song_schema)
{

}

const std::string& song::path() const
{
	return m_path;
}

auto song::schema() const -> schema_type
{
	return m_schema;
}

std::string song::to_string() const
{
	if(m_schema == schema_type::file)
		return m_path;
	else
		return "youtube://" + m_path;
}
