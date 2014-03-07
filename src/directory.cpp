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

#include <boost/filesystem.hpp>
#include <algorithm>
#include "directory.h"

using lock_guard = std::lock_guard<std::mutex>;
using namespace boost::filesystem;

directory::directory(path_type full_path)
: m_path(std::move(full_path)), m_mutex(new std::mutex()), m_loaded(false)
{
	m_name = m_path.end();
	--m_name;
}

directory::directory(path_type full_path, only_for_search)
: m_path(std::move(full_path))
{
	m_name = m_path.end();
	--m_name;
}

auto directory::name() const -> const path_type&
{
	return *m_name;
}

auto directory::path() const -> const path_type&
{
	return m_path;
}

auto directory::directories() const -> const directories_list&
{
	load();
	return m_directories;
}

auto directory::files() const -> const files_list&
{
	load();
	return m_files;
}

const directory& directory::find_directory(path_type::iterator start, 
	path_type::iterator end) const
{
	if(start == end)
		return *this;
	else {
		load();
		auto to_search = directory(*start, only_for_search());
		auto iter = std::lower_bound(
			m_directories.begin(),
			m_directories.end(),
			to_search
		);
		if(iter == m_directories.end() || iter->name() != to_search.name())
			throw std::runtime_error("path not found");
		++start;
		return iter->find_directory(start, end);
	}
}

void directory::load() const
{
	lock_guard _(*m_mutex);
	if(m_loaded)
		return;
	directory_iterator end;
  	for(directory_iterator iter(m_path); iter != end; ++iter) {
  		if (is_directory(*iter)) {
        	m_directories.emplace_back(iter->path());
     	}
     	// TODO: move the extension check to some object
     	else if(is_regular_file(*iter) && extension(*iter) == "mp3") {
     		m_files.emplace_back(iter->path().filename().string());
     	}
   	}
   	std::sort(m_directories.begin(), m_directories.end());
   	std::sort(m_files.begin(), m_files.end());
	m_loaded = true;
}

void directory::add_directory(directory dir)
{
	m_directories.push_back(std::move(dir));
}

std::string directory::path_for_file(const std::string& file_name) const
{
	return (m_path / file_name).string();
}

bool operator<(const directory& lhs, const directory& rhs)
{
	return lhs.name() < rhs.name();
}
