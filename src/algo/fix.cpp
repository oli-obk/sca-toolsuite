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
#include "io.h"
#include "stack_algorithm.h"

template<class AvalancheContainer, class Logger>
void run(std::vector<int>& grid, const dimension& dim, int hint=-1)
{
	AvalancheContainer container(dim.area_without_border());
	Logger logger(stdout);
	if(hint == -1)
	{
		fix(grid, dim, container, logger);
	}
	else
	 fix(grid, dim, human2internal(hint, dim.width()), container, logger);
}

class MyProgram : public Program
{
	int main()
	{
		FILE* read_fp = stdin;
		int hint = -1;
		char output_type = 's';

		switch(argc) {
			case 3: hint = atoi(argv[2]);
			case 2:
				output_type = argv[1][0];
				assert_usage(!argv[1][1] &&
					(output_type=='l'||output_type=='s'));
				break;
			default:
				exit_usage();
				return 1;
		}

		std::vector<int> grid;
		dimension dim;

		read_grid(read_fp, &grid, &dim);

		switch(output_type) {
			case 'l': ::run<sandpile::array_stack,
					sandpile::fix_log_l>(
					grid, dim, hint);
				break;
		//	case 'h': run<ArrayStack, FixLogLHuman>(grid, dim, hint); break;
			case 's':
				::run<sandpile::array_stack,
					sandpile::fix_log_s>(
					grid, dim, hint);
				write_grid(stdout, &grid, &dim);
				break;
		}
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Runs the stabilisation algorithm until grid is stable.\n"
		"Algorithm runs correctly on every configuration >= 0.";
	help.input = "input grid";
	help.syntax = "algo/fix s|l [<hint>]";
	help.add_param("s|l", "s calculates resulting grid, l the number each cell fires");
	help.add_param("<hint>", "only ensures that cell at hint will be fired");

	MyProgram program;
	return program.run(argc, argv, &help);
}


