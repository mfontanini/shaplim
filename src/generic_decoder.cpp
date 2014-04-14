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

#include <stdexcept>
#include "generic_decoder.h"
#include "song_stream.h"

int read_function(void* opaque, uint8_t* buf, int buf_size) 
{
    auto& stream = *reinterpret_cast<song_stream*>(opaque);
    if(!stream.bytes_left())
        return 0;
    auto available = stream.available();
    size_t bytes_to_copy = std::min(available, static_cast<size_t>(buf_size));
    std::copy(
        stream.buffer_ptr(),
        stream.buffer_ptr() + bytes_to_copy,
        buf
    );
    stream.advance(bytes_to_copy);
    return bytes_to_copy;
}

int64_t seek_function(void* opaque, int64_t offset, int whence)
{
    auto& stream = *reinterpret_cast<song_stream*>(opaque);
    if(whence == AVSEEK_SIZE) {
        auto size = stream.size();
        if(size > 0) {
            return size;
        }
    }
    if(whence == SEEK_END)
        return -1;
    if(stream.bytes_left()) {
        std::cout << "WHence: " << whence << " - Offset: " << offset << " -> " << stream.current_offset() << std::endl;
        stream.seek(offset);
        return 1;
    }
    else
        return -1;
}

generic_decoder::generic_decoder()
: m_frame(avcodec_alloc_frame(), &av_free)
{
	static std::once_flag flag;
	std::call_once(flag, av_register_all);

	av_init_packet(&m_packet);
}

void generic_decoder::decode(song_stream stream, types::decode_buffer_type &buffer)
{
	m_running = true;

	const std::shared_ptr<AVIOContext> avioContext(
        avio_alloc_context(
            reinterpret_cast<unsigned char*>(av_malloc(8192)),
            8192, 
            0, 
            &stream, 
            &read_function, 
            nullptr, 
            &seek_function
        ), 
        &av_free
    );

    auto av_formatPtr = avformat_alloc_context();
    av_formatPtr->pb = avioContext.get();
    int err_code = avformat_open_input(&av_formatPtr, "dummy", nullptr, nullptr);
    if(err_code != 0) {
        char error[512];
        av_strerror(err_code, error, sizeof(error));
        throw std::runtime_error(error);
    }
    const auto av_format = std::shared_ptr<AVFormatContext>(av_formatPtr, &avformat_free_context);
    
    int stream_id = -1;
    for (size_t i = 0; i < av_format->nb_streams; ++i) {
        if (av_format->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream_id = i;
            break;
        }
    }
    if(stream_id == -1) 
        throw std::runtime_error("Audio stream not found");
    
    AVCodecContext *ctx = av_format->streams[stream_id]->codec;
    AVCodec *codec = avcodec_find_decoder(ctx->codec_id);

    if(codec == nullptr) {
        throw std::runtime_error("Failed to find codec.");
    }
    if(avcodec_open2(ctx, codec, nullptr) < 0) {
        throw std::runtime_error("Failed to open codec.");
    }
    
    m_on_rate_change(ctx->sample_rate);

    while(m_running && av_read_frame(av_format.get(), &m_packet) >= 0)
    {
        if(m_packet.stream_index == stream_id) {
            int frame_decoded = 0;
            avcodec_decode_audio4(ctx, m_frame.get(), &frame_decoded, &m_packet);
            if(frame_decoded){
                if(ctx->sample_fmt == AV_SAMPLE_FMT_S16) {
                    buffer.put(
                        (const uint16_t*)m_frame->extended_data[0], 
                        (const uint16_t*)m_frame->extended_data[0] + m_frame->nb_samples * ctx->channels
                    );
                }
                else {
                    throw std::runtime_error("Needs resampling");
                }
            }

        }
    }
}

void generic_decoder::stop_decode()
{
	m_running = false;
}

float generic_decoder::percent_so_far()
{
	return 0;
}
