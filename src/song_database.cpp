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

#include <iterator>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/attachedpictureframe.h>
#include "song_database.h"

song_information::song_information(const std::string& file_name)
: m_length()
{
	TagLib::MPEG::File f(file_name.c_str());
    auto tag = f.tag();
    m_artist = tag->artist().to8Bit(true);
    m_title = tag->title().to8Bit(true);
    m_album = tag->album().to8Bit(true);
    if(f.audioProperties())
    	m_length = std::chrono::seconds(f.audioProperties()->length());
    if(f.ID3v2Tag()) {
        const TagLib::ID3v2::FrameList& l = f.ID3v2Tag()->frameListMap()["APIC"];
        if(!l.isEmpty()) {
            using TagLib::ID3v2::AttachedPictureFrame;

            auto picture_frame = static_cast<AttachedPictureFrame*> (*l.begin());
            const auto& image = picture_frame->picture();
            m_picture_mime = picture_frame->mimeType().to8Bit();
            using base64_text =	boost::archive::iterators::base64_from_binary<
				boost::archive::iterators::transform_width<
    				const char *,
    				6,
    				8
				>
			>;
			std::copy(
				base64_text(image.data()),
				base64_text(image.data() + image.size()),
				std::back_inserter(m_picture)
			);
            auto padding = image.size() % 3;
            if(padding != 0)
                m_picture.insert(m_picture.end(), 3 - padding, '=');
        }
    }
    if(m_artist.empty())
    	m_artist = "Unknown";
   	if(m_title.empty())
    	m_title = "Unknown";
   	if(m_album.empty())
    	m_album = "Unknown";
}

const std::string& song_information::artist() const
{
	return m_artist;
}

const std::string& song_information::album() const
{
	return m_album;
}

const std::string& song_information::title() const
{
	return m_title;
}

const std::string& song_information::picture() const
{
	return m_picture;
}

const std::string& song_information::picture_mime() const
{
	return m_picture_mime;
}

const std::chrono::seconds& song_information::length() const
{
	return m_length;
}

// *******************
// ** song_database **
// *******************

using lock_type = std::lock_guard<std::mutex>;

const song_information& song_database::song_info(const std::string& path)
{
	lock_type _(m_lock);
	auto iter = m_db.find(path);
	if(iter == m_db.end()) {
		iter = m_db.insert(
			std::make_pair(path, song_information(path))
		).first;
	}
	return iter->second;
}
