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

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include "general.h"
#include "io.h"

class MyProgram : public Program
{
	int main()
	{
		std::vector<int> grid;
		dimension dim;
		int predefined_value = 0;

		switch(argc)
		{
			case 4: predefined_value = atoi(argv[3]);
			case 3:
				dim.width = atoi(argv[1])+2;
				dim.height = atoi(argv[2])+2;
				break;
			default:
				exit_usage();
		}

		create_empty_grid(&grid, &dim, predefined_value);
		write_grid(stdout, &grid, &dim);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Dumps a new grid";
	help.syntax = "core/create <width> <height> [<initial>]";
	help.add_param("initial", "intial value for all cells, defaults to 0");

	MyProgram program;
	return program.run(argc, argv, &help);
}
