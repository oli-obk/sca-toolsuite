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

//! @file ca_convert.h Routines to convert between ca formats

#ifndef CA_CONVERT_H
#define CA_CONVERT_H

#include "ca.h"

namespace sca { namespace ca {

enum class type
{
	formula,
	table,
	grids
};

template<type in, type out>
class converter
{
	dont_instantiate_me_id<type, in> d;
};

template<type T>
class converter<T, T>
{
	void operator()(std::istream& i, std::ostream& o){
		o << i.rdbuf();
	}
};

template<type T>
class converter<type::table, type::grids>
{
	void operator()(std::istream& i, std::ostream& o) {
		_ca_table_hdr_t tbl_hdr(i);
		(void)tbl_hdr(i);
		dont_instantiate_me_id<type, in>();
	}
};

}}

#endif // CA_CONVERT_H
