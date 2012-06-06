/*************************************************************************/
/* sca toolsuite - a toolsuite to simulate sandpile cellular automata.   */
/* Copyright (C) 2011-2012                                               */
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

#include "general.h"
#include "stack_algorithm.h"
#include "io.h"
inline void stabilize(std::vector<int>* grid, const dimension* dim)
{
	// +1 is an ugly, necessary trick
	ArrayStack container(dim->area_without_border() + 1);
	FixLogS logger(NULL);
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

#endif // ASM_BASIC_H
