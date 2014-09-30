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

#include <cstring>
#include <cassert> // TODO
#include <string>

#include "gridfile.h"

sca::io::color_t::color_t(const char *str)
{ // TODO: const member construction...
	switch(*str)
	{ // TODO: not use assert!
		case 'g': assert(!strncmp(str, "green", 5)); g = 255; break;
		case 'r': assert(!strncmp(str, "red", 3)); r = 255; break;
		case 'b': assert(!strncmp(str, "blue", 4)); b = 255; break;
		default: throw "Unknown color type";
	}
}

sca::io::path_node::markup::markup(const char *str) :
	color(str)
{ // TODO: const constr
	const char* ptr = str;
	next_word(ptr);
	tp = (!strcmp(ptr, "active"))
		? mark_type::active
		: mark_type::formula;
	if(tp == mark_type::formula)
	 formula = ptr;
}

sca::io::path_node::arrow::arrow(const char *str)
{
	const char* ptr = str;
	p1.x = atoi(ptr); next_word(ptr);
	p1.y = atoi(ptr); next_word(ptr);
	p2.x = atoi(ptr); next_word(ptr);
	p2.y = atoi(ptr);
}

bool sca::io::path_node::parse(sca::io::secfile_t &inf)
{
	/*if(inf.read_int(grid_id))
		{
			inf.read_newline();*/
	std::string str;
	while((str = inf.read_string()).size())
	{
		if(!str.compare(0, 4, "mark"))
		 markup_list.emplace_back(str.data() + 5);
		else if(!str.compare(0, 5, "arrow"))
		 arrow_list.emplace_back(str.data() + 6);
		else
		{
			assert(description.empty());
			description = str;
		}
	}
	return true;
	//}
	//else return false;
}



