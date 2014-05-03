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

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include "general.h"
#include "geometry.h"

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
				dim = dimension(atoi(argv[1]),
					atoi(argv[2]));
				break;
			default:
				exit_usage();
				return 1; // omit warnings
		}

		std::cout << grid_t(dim, 0, predefined_value);
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
