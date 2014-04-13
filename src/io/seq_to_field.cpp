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

#include <cstdlib>
#include <cstdio>
#include <vector>

#include "general.h"
#include "geometry.h"

class MyProgram : public Program
{
	int main()
	{
		FILE* read_fp=stdin;
		dimension dim;

		switch(argc)
		{
			case 3: dim.width = atoi(argv[1]) + 2;
				dim.height =  atoi(argv[2]) + 2;
				break;
			default: exit_usage();
		}

		const int grid_size = dim.area_without_border();
		std::vector<int> grid(dim.area());

		int symbols_read;
		int index;

		do {
			symbols_read = fscanf(read_fp, "%d", &index);
			if(symbols_read > 0) {
				if(!human_idx_on_grid(grid_size, index))
				 exit("You must assert for each index i: 0 <= i < area.");
				grid[human2internal(index, dim.width)]++;
			}
		} while(symbols_read > 0);

		write_grid(stdout, &grid, &dim);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Iterates through a given sequence of numbers, adding a coordinate to\n"
		"each such a position on a grid. Adding is meant without any stabilization.";
	help.input = "sequence of numbers";
	help.output = "resulting grid";
	help.syntax = "io/seq_to_field <width> <height>";
	help.add_param("<width>, <height>", "specifies the desired dimension for the output grid");

	MyProgram program;
	return program.run(argc, argv, &help);
}
