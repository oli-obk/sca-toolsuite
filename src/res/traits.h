/*************************************************************************/
/* sca toolsuite - a toolsuite to simulate cellular automata.            */
/* Copyright (C) 2011-2014                                               */
/* Johannes Lorenz                                                       */
/* https://github.com/JohannesLorenz/sca-toolsuite                       */
/*                                                                       */
/* This program is free software; you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation; either version 3 of the License, or (at */
/* your option) any later version.                                       */
/* This program is distributed in the hope that it will be useful, but   */
/* WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      */
/* General Public License for more details.                              */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program; if not, write to the Free Software           */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA  */
/*************************************************************************/

#ifndef TRAITS_H
#define TRAITS_H

#include <type_traits>
#include <cstdint>

template<class T>
struct area_class { using type = unsigned int; };
template<>
struct area_class<char> { using type = unsigned short; };
template<>
struct area_class<int8_t> { using type = unsigned short; };
template<>
struct area_class<uint8_t> { using type = unsigned short; };

//! @arg Coord The type for using coords. signed or unsigned
template<class Coord, class Area = typename area_class<Coord>::type> // TODO: def for area
struct coord_traits
{
	using coord_t = Coord;
	using u_coord_t = typename std::make_unsigned<coord_t>::type; // TODO...
	using area_t = typename std::make_unsigned<Area>::type;
};

template<class Cell>
struct cell_traits
{
	using cell_t = Cell;
};

using def_coord_traits = coord_traits<int>;
using def_cell_traits = cell_traits<int>;
using def_cell_const_traits = cell_traits<const int>;

#endif // TRAITS_H
