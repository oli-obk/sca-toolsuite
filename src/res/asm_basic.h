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

#ifndef ASM_BASIC_H
#define ASM_BASIC_H

#include "geometry.h"
#include "stack_algorithm.h"
#include "io.h"

namespace sandpile
{

inline void stabilize(std::vector<int>* grid, const dimension* dim)
{
	// +1 is an ugly, necessary trick
	array_stack container(dim->area_without_border() /*+ 1*/);
	fix_log_s logger(nullptr);
	fix(grid, dim, &container, &logger);
}

//! Given an empty vector @a grid, creates grid of dimension @a dim
//! with all cells being @a predefined_value
inline void get_identity(std::vector<int>* grid, const dimension* dim)
{
	create_empty_grid(grid, dim, 6);
	stabilize(grid, dim);
	for(std::vector<int>::iterator itr = grid->begin();
		itr != grid->end(); itr++)
		if(*itr>=0)
		*itr = 6 - (*itr) ;
	stabilize(grid, dim);
}

//! calculates superstabilization of @a grid
inline void superstabilize(std::vector<int>* grid, const dimension* dim,
	const std::vector<int>* identity)
{
	assert(identity->size() == grid->size());

	stabilize(grid, dim);

	unsigned int area = dim->area();
	for(unsigned int i = 0; i < area; i++)
	 if((*grid)[i]>=0)
	  (*grid)[i] = 3 - (*grid)[i] + (*identity)[i];

	stabilize(grid, dim);

	for(unsigned int i = 0; i < area; i++)
	 if((*grid)[i]>=0)
	  (*grid)[i] = 3 - (*grid)[i];
}

//! calculates superstabilization of @a grid
inline void superstabilize(std::vector<int>* grid, const dimension* dim)
{
	std::vector<int> identity;
	get_identity(&identity, dim);
	superstabilize(grid, dim, &identity);
}

}

#endif // ASM_BASIC_H
