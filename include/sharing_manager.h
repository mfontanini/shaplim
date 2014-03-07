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

#ifndef SHAPLIM_SHARING_MANAGER_H
#define SHAPLIM_SHARING_MANAGER_H

#include <vector>
#include <string>
#include "directory.h"

class sharing_manager {
public:
	sharing_manager(const std::vector<std::string>& shared_dirs);

	std::vector<std::string> shared_directories();
	const directory& find_directory(const std::string& full_path) const;
private:
	directory m_virtual_root;
};

#endif // SHAPLIM_SHARING_MANAGER_H
