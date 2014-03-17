/*************************************************************************/
/* sca toolsuite - a toolsuite to simulate cellular automata         .   */
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

#include <cstdlib>
#include <cstdio>
#include <vector>

#include "general.h"
#include "io.h"
#include "stack_algorithm.h"

int main(int argc, char** argv)
{
	(void)argv;
	FILE* read_fp = stdin;
	if(argc>1) {
		fputs("There are no arguments for this tool.\n", stderr);
		exit(1);
	}

	std::vector<int> grid;
	dimension dim;
	read_grid(read_fp, &grid, &dim);

	std::vector<int> workGrid;
	unsigned int area = dim.area_without_border();
	ArrayQueue stack(area);
	fwrite(&(dim.width),4,1,stdout);
	fwrite(&(dim.height),4,1,stdout);
	for(unsigned int i=0; i<area; i++)
	{
		workGrid = grid;
		int idx = human2internal(i, dim.width);
		while(++workGrid[idx]<4);

		l_hint(&workGrid, &dim, idx, &stack, stdout);
	}

	return 0;
}


