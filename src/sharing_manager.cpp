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

#include "sharing_manager.h"

sharing_manager::sharing_manager(const std::vector<std::string>& shared_dirs)
: m_virtual_root(make_virtual_directory(shared_dirs.begin(), shared_dirs.end()))
{

}

std::vector<std::string> sharing_manager::shared_directories()
{
	std::vector<std::string> output;
	for(const auto& dir : m_virtual_root.directories())
		output.push_back(dir.name().string());
	return output;
}

const directory& sharing_manager::find_directory(const std::string& full_path) const
{
	directory::path_type dir_path(full_path);
	return m_virtual_root.find_directory(dir_path.begin(), dir_path.end());
}
