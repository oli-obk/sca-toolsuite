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

#ifndef STRINGS_H
#define STRINGS_H

#include <string>
#include <vector>
#include <set>
#include <algorithm>

namespace sca { namespace util {

// TODO: source: http://stackoverflow.com/
// questions/8581832/converting-a-vector-to-string
template<template<class> class Cont, class T>
std::string to_string(const Cont<T>& cont) {

	std::ostringstream oss;

	if (!cont.empty())
	{
		// Convert all but the last element to avoid a trailing ","
		std::copy(vec.begin(), vec.end()-1,
			std::ostream_iterator<int>(oss, ", "));

		// Now add the last element with no delimiter
		oss << vec.back();
	}

	return oss.str();
}

} }

#endif // STRINGS_H
