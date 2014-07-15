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

//! @file routines for printing containers with ostream

#ifndef PRINT_H
#define PRINT_H

#include <iostream>
#include <set>
#include "geometry.h"

template <class T>
class has_cbegin
{
	typedef char one; // TODO: remove these, use uint8_t
	typedef long two;

	template <typename C> static one test( decltype(&C::cbegin) ) ;
	template <typename C> static two test(...);
public:
	enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

// TODO: own h and cpp files
template<class Cont>
class print_cont
{
	template<class T, typename std::enable_if<!has_cbegin<T>::value>::type* = nullptr>
	static std::ostream& _dump(std::ostream& stream, T& elem)
	{
		return stream << elem;
	}
	template<class T, typename std::enable_if<has_cbegin<T>::value>::type* = nullptr>
	static std::ostream& _dump(std::ostream& stream, T& elem)
	{
		if(elem.empty())
		 stream << "empty-container";
		else
		{
			stream << "container(";
			for(const auto& x : elem)
			 stream << print_cont<typename T::value_type>(x) << ", ";
			stream << ")";
		}
		return stream;
	}
	const Cont& c;
public:
	print_cont(const Cont& c) : c(c) {}
	friend std::ostream& operator<< (std::ostream& stream,
		const print_cont& pc)
	{
		return _dump(stream, pc.c);
	}
};

template<class First, class Second>
class print_cont<std::pair<First, Second>>
{
	using Cont = std::pair<First, Second>;
	const Cont& c;
public:
	print_cont(const Cont& c) : c(c) {}
	friend std::ostream& operator<< (std::ostream& stream,
		const print_cont& pc)
	{
		return stream << "pair(" << print_cont<First>(pc.c.first)
			<< ", " << print_cont<Second>(pc.c.second) << ")";
	}
};

template<class ...Args>
struct print_cont<std::set<_point<Args...>>>
{
	using Cont = std::set<_point<Args...>>;
	const Cont& c;
public:
	print_cont(const Cont& c) : c(c) {}
	friend std::ostream& operator<< (std::ostream& stream,
		const print_cont& pc)
	{
		if(pc.c.empty())
		 stream << "(no points)";
		else
		{
		int skipped = 0;
		typename Cont::iterator itr2;
		for(auto itr = pc.c.begin(); itr != pc.c.end(); ++itr)
		{
			itr2 = itr;
			++itr2;
			if(itr2 != pc.c.end() && itr2->y == itr->y && itr2->x == itr->x + 1)
			 ++skipped;
			else
			{
				if(skipped == 0)
					stream << *itr;
				else
				{
					stream << "(" << (int)itr->x-skipped
					       << ".." << (int)itr->x
					       << ", " << (int)itr->y << ")";
					skipped = 0;
				}

				if(itr2 != pc.c.end())
				 stream << ", ";
			}
		}
		}
		return stream;
	}
};

template<class Cont>
print_cont<Cont> mk_print(const Cont &c) { return print_cont<Cont>(c); }

#endif // PRINT_H
