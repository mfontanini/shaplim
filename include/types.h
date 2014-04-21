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

#ifndef SHAPLIM_TYPES_H
#define SHAPLIM_TYPES_H

#include "ring_buffer.h"

namespace types {
	//using decode_buffer_type = ring_buffer<short, 8192>;
	using decode_buffer_type = lock_free_ring_buffer<short, 8192>;
}

#endif // SHAPLIM_TYPES_H
