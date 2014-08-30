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
#include <iostream>

#include "general.h"
#include "io.h"
#include "stack_algorithm.h"

template<class AvalancheContainer, class Logger>
void run(grid_t& grid, int hint=-1)
{
	AvalancheContainer container(grid.human_dim().area());
	Logger logger(stdout);
	if(hint == -1)
	{
		fix(grid.data(), grid.internal_dim(), container, logger);
	}
	else
	 fix(grid.data(), grid.internal_dim(), human2internal(hint, grid.internal_dim().width()), container, logger);
}

class MyProgram : public Program
{
	exit_t main()
	{
		std::istream& read_fp = std::cin;
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
				return exit_usage();
		}

		grid_t grid(read_fp, 1);

		switch(output_type) {
			case 'l': ::run<sandpile::array_stack,
					sandpile::fix_log_l>(
					grid, hint);
				break;
		//	case 'h': run<ArrayStack, FixLogLHuman>(grid, dim, hint); break;
			case 's':
				::run<sandpile::array_stack,
					sandpile::fix_log_s>(
					grid, hint);
				std::cout << grid;
				break;
		}
		return exit_t::success;
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


