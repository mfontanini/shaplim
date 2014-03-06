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

#ifndef SHAPLIM_DIRECTORY_H
#define SHAPLIM_DIRECTORY_H

#include <string>
#include <mutex>
#include <set>
#include <memory>
#include <boost/filesystem/path.hpp>
#include "music_file.h"

class directory {
public:
	using directories_list = std::vector<directory>;
	using files_list = std::vector<music_file>;
	using path_type = boost::filesystem::path;

	directory(path_type full_path);

	const path_type& name() const;
	const path_type& path() const;
	const directories_list& directories();
	const files_list& files();

	void load();
	std::string path_for_file(const std::string& file_name);
	directory& find_directory(const std::string& full_path);
	directory& find_directory(path_type::iterator start, 
		path_type::iterator end);
private:
	struct only_for_search{};

	directory(path_type full_path, only_for_search);
	template<typename InputIterator>
	directory(InputIterator start, InputIterator end);

	void add_directory(directory dir);
	template<typename InputIterator>
	friend directory make_virtual_directory(InputIterator start, 
		InputIterator end);

	path_type m_path;
	boost::filesystem::path::iterator m_name;
	directories_list m_directories;
	files_list m_files;
	mutable std::shared_ptr<std::mutex> m_mutex;
	mutable bool m_loaded;
};

template<typename InputIterator>
directory make_virtual_directory(InputIterator start, InputIterator end)
{
	return directory(std::move(start), std::move(end));
}

template<typename InputIterator>
directory::directory(InputIterator start, InputIterator end)
: m_mutex(new std::mutex())
{
	while(start != end) {
		add_directory(directory(*start));
		++start;
	}
	std::sort(m_directories.begin(), m_directories.end());
	m_loaded = true;
}

bool operator<(const directory& lhs, const directory& rhs);

#endif // SHAPLIM_DIRECTORY_H

