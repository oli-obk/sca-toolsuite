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

#include <cstdio>
#include <vector>

#include "general.h"
#include "grid.h"

class MyProgram : public Program
{
	exit_t main()
	{
		std::vector<int> grid;
		dimension dim;
		read_grid(stdin, &grid, &dim);
		const int grid_size = dim.area_without_border();
		int idx;
		for(int i = 1; i < argc; i++)
		{
			idx = atoi(argv[i]);
			if(! human_idx_on_grid(grid_size, idx))
			 exit("You must assert for each index i: 0 <= i < area.");
			grid[human2internal(atoi(argv[i]),dim.width())]++;
		}

		write_grid(stdout, &grid, &dim);
		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Throws grains on specified cells of the grid.\n"
		"Throwing is meant without any stabilization.";
	help.syntax = "math/add <i1> <i2> ... <in>";
	help.add_param("<i1> <i2> ... <in>", "Numbers to be addded. 0 <= n.");

	MyProgram program;
	return program.run(argc, argv, &help);
}

