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

class MyProgram : public Program
{
	int main()
	{
		if(argc != 3)
		 exit_usage();

		switch(argc)
		{
			case 2:

			case 1:

			default:
				exit_usage();
		}

		std::vector<int> grid, result;
		dimension dim;
		std::vector <int> filter;
		dimension dim_filter;

		read_array(stdin, &grid, &dim);
		result.resize(grid.size(), 0);
		get_input(argv[2]);
		read_array(stdin, &filter, &dim_filter);

		if(dim_filter.width % 2 || dim_filter.height % 2)
		 exit("Filter matrix must have odd width and height.");

		int centerx = dim_filter.width >> 1;
		int centery = dim_filter.height >> 1;

		for(unsigned int y = 0; y < dim.height; y++)
		for(unsigned int x = 0; x < dim.width; x++)
		// iterate over input grid
		{
			const int internal = x+y*dim.width;
			if(grid[internal] == INT_MIN) { // excludes border
				result[internal] = INT_MIN;
			}
			else {
				eqsolver::ast_print solver(dim.height, dim.width, x-1,y-1,&grid[internal]);
				result[internal] = solver(ast);
			}


		}

		write_array(stdout, &grid, &dim);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "io/filter <shell command>";
	help.description = "Reads grid from stdin and writes it to stdout. Useful for formatting.";
	help.input = "input grid, or none if a file was given as an argument";
	help.output = "the same grid";
	help.add_param("infile", "specifies a file to read a grid from");

	MyProgram p;
	return p.run(argc, argv, &help);
}


