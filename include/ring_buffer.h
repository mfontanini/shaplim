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

#ifndef SHAPLIM_RING_BUFFER_H
#define SHAPLIM_RING_BUFFER_H

#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <array>
#include <iostream> // borrame

template<typename T, size_t n>
class ring_buffer {
public:
	ring_buffer();

	template<typename InputIterator>
	bool put(InputIterator start, InputIterator end);

	template<typename OutputIterator>
	void get(OutputIterator output, size_t count, T default_value = T());
private:
	static constexpr size_t buffer_size = n;
	using buffer_type = std::array<T, buffer_size>;
	using locker_type = std::unique_lock<std::mutex>;
	using iterator = decltype(std::declval<buffer_type>().begin());

	buffer_type m_buffer;
	iterator m_front, m_back;
	bool m_empty;
	std::mutex m_lock;
	std::condition_variable m_cond;
};

template<typename T, size_t n>
ring_buffer<T, n>::ring_buffer()
: m_front(std::begin(m_buffer)), m_back(m_front), m_empty(true)
{

}

template<typename T, size_t n>
template<typename InputIterator>
bool ring_buffer<T, n>::put(InputIterator start, InputIterator end)
{
	auto size = std::distance(start, end);
	locker_type locker(m_lock);
	while(size != 0) {
		if(m_front == m_back && !m_empty) {
			m_cond.wait(locker);
			if(m_front == m_back && !m_empty)
				return false;
		}
		auto buffer_end = (m_back > m_front) ? m_back : std::end(m_buffer);
		auto space_left = std::distance(m_front, buffer_end);
		auto amount_to_write = std::min(space_left, size);
		m_front = std::copy(start, start + amount_to_write, m_front);
		size -= amount_to_write;
		start += amount_to_write;
		if(m_front == std::end(m_buffer))
			m_front = std::begin(m_buffer);
		m_empty = false;
	}
	m_cond.notify_one();
	return true;
}

template<typename T, size_t n>
template<typename OutputIterator>
void ring_buffer<T, n>::get(OutputIterator output, size_t count, T default_value)
{
	locker_type locker(m_lock);
	while(count != 0) {
		if(m_front == m_back && m_empty) {
			std::fill(output, output + count, default_value);
			break;
		}
		auto buffer_end = (m_front > m_back) ? m_front : std::end(m_buffer);
		auto space_left = std::distance(m_back, buffer_end);
		auto amount_to_read = std::min(static_cast<size_t>(space_left), count);
		output = std::copy(m_back, m_back + amount_to_read, output);
		count -= amount_to_read;
		m_back += amount_to_read;
		if(m_back == std::end(m_buffer))
			m_back = std::begin(m_buffer);
		if(m_back == m_front)
			m_empty = true;
	}
	m_cond.notify_one();
}

#endif // SHAPLIM_RING_BUFFER_H
