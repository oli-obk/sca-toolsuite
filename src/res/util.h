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

#ifndef UTILITY_H
#define UTILITY_H

namespace sca { namespace util {

/*
	templates for structs that you don't want to instantiate
*/

template<typename ...> struct falsify : public std::false_type { };
template<typename T, T Arg> class falsify_id : public std::false_type { };
template<typename ...Args>
class dont_instantiate_me {
	static_assert(falsify<Args...>::value, "This should not be instantiated.");
};
template<typename T, T Arg>
class dont_instantiate_me_id {
	static_assert(falsify_id<T, Arg>::value, "This should not be instantiated.");
};

}}

#endif // UTILITY_H
