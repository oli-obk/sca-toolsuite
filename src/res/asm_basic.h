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

inline void stabilize(grid_t& grid)
{
	// +1 is an ugly, necessary trick
	array_stack container(grid.human_dim().area() /*+ 1*/);
	fix_log_s logger(nullptr);
	fix(grid.data(), grid.internal_dim(), container, logger);
}

//! Given an empty vector @a grid, creates grid of dimension @a dim
//! with all cells being @a predefined_value
inline grid_t get_identity(const dimension& dim)
{
	grid_t grid(dim, 1, 6); // grid with every cell = 6
	stabilize(grid);
	for(cell_t& c : grid)
	 c = 6 - c;
	stabilize(grid);
	return grid;
}

/*inline void get_identity(std::vector<int>& grid, const dimension& dim)
{
	create_empty_grid(grid, dim, 6);
	stabilize(grid, dim);
	// TODO: use range based for everywhere
	for(std::vector<int>::iterator itr = grid.begin();
		itr != grid.end(); itr++)
		if(*itr>=0)
		*itr = 6 - (*itr) ;
	stabilize(grid, dim);
}*/

//! calculates superstabilization of @a grid
inline void superstabilize(grid_t& grid,
	const grid_t& identity)
{
	assert(identity.internal_dim() == grid.internal_dim());

	stabilize(grid);

	for(const point& p : grid.points())
	if(grid[p]>=0) // todo: necessary?
	 grid[p] = 3 - grid[p] + identity[p];

	stabilize(grid);

	for(const point& p : grid.points())
	 if(grid[p]>=0)
	  grid[p] = 3 - grid[p];

	// TODO: mention holroyd  et al as source
}

//! calculates superstabilization of @a grid
inline void superstabilize(grid_t& grid)
{
	superstabilize(grid, get_identity(grid.human_dim()));
}

}

#endif // ASM_BASIC_H
