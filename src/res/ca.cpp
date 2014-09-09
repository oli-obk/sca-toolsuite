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

// TODO: rename this file: ca_table.cpp
#include "ca_table.h"

namespace sca { namespace ca {

constexpr const uint32_t tbl_detail::version_t::id;

tbl_detail::size_check::size_check(int size)
{
	if(size < 0)
	 throw "Error: Size negative (did you set num_states == 0?).";
	if(size > (1<<18))
	 throw "Error: This ca is too large for a table.";
}

tbl_detail::header_t::header_t(std::istream &stream)
{
	char buf[9];
	buf[8] = 0;
	stream.read(buf, 8);
	if(strcmp(buf, "ca_table"))
	 throw "Error: This file has no ca_table header.";
}

tbl_detail::version_t::version_t(const uint32_t &i)
{
	if(i != id)
	{
		//char err_str[] = "Incompatible versions:\n" // 23
		//	"file: ................,\n" // 6 + 16 + 2
		//	"expected: ................";
		//sprintf(err_str + 23 + 6, "%016d", i);
		//sprintf(err_str + 23 + 6 + 16 + 2 + 10, "%016d", id);
		throw std::string("Incompatible versions: "
				  "read: " + std::to_string(i) +
				  ", expected: " + std::to_string(id) + ".");
	}
}

} }
