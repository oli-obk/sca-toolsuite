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

#ifndef NAME_TYPE_MAP_H
#define NAME_TYPE_MAP_H

namespace sca {

// TODO: inside name_type_map_t?
template<class Enum>
struct name_type_pair
{
	const char* name;
	Enum e;
};

template<std::size_t N, class Enum, Enum False>
struct name_type_map_t
{
	// TODO: binary tree using map's stack allocator?
	name_type_pair<Enum> map[N];

	using pair_t = name_type_pair<Enum>;

	// TODO: private stuff?
	Enum operator[](const char* _name) const
	{
		const pair_t* const end = map + N;
		Enum result = False;
		for(const pair_t* ptr = map;
			result == False && ptr != end; ++ptr)
		if(!strcmp(ptr->name, _name))
		 result = ptr->e;
		return result;
	}

	void dump_names(std::ostream& stream) const
	{
		const pair_t* const end = map + N;
		for(const pair_t* ptr = map; ptr != end; ++ptr)
		 stream << " * " << ptr->name << std::endl;
	}
};

}

#endif // NAME_TYPE_MAP_H
