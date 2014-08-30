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
#include "rotor_algorithm.h"
#include "asm_basic.h"

template<class AvalancheContainer, class Logger>
void run(grid_t& grid, grid_t& chips, int hint=-1)
{
	AvalancheContainer container(grid.human_dim().area());
	Logger logger(stdout);
	if(hint == -1)
	{
		/*for(unsigned int i=0; i<dim.area_without_border(); i++)
		{
			const int internal = human2internal(i, dim.width);
			rotor_fix(&grid, &chips, &dim, internal, &container, &logger);
		}*/
		//fix(&chips, &dim, &container, &logger);
		sandpile::superstabilize(chips); // TODO: should this not be in rotor algo . h?
		// -> TODO: does this not need to write logs, too?
		rotor::rotor_fix_naive(grid, chips, container, logger); // TODO: is naive wanted??
	}
	else
	 rotor::rotor_fix(grid, chips, human2internal(hint, grid.internal_dim().width()), container, logger);
}

class MyProgram : public Program
{
	exit_t main()
	{
		std::istream& read_fp = std::cin;
		int hint = -1;
		char output_type = 's';
		const char* shell_command = NULL;

		switch(argc) {
			case 4: hint = atoi(argv[3]);
			case 3: shell_command = argv[2];
				output_type = argv[1][0];
				if(argv[1][1] || !(output_type=='l'||output_type=='s'))
				 exit_usage();
				break;
			default:
				return exit_usage();
		}

	//	std::vector<int> grid;
	//	dimension dim;

		//read_grid(read_fp, &grid, &dim);
		grid_t grid(read_fp, 1);
	/*	for(unsigned int i=0;i<dim.area();i++)
		 if(!is_border(dim, i))
		  grid[i]&=3;*/
		for(auto& c : grid)
		 c&=3;

		/*std::vector<int> chips;
		dimension dim2;*/
		get_input(shell_command);

		grid_t chips(stdin, 1); // TODO: this will not work with cin somehow, because of get_input()

		//read_grid(read_fp, &chips, &dim2);

		if(grid.internal_dim() != chips.internal_dim())
		 exit("Different dimensions in both grids are not allowed.");

		switch(output_type) {
			// TODO: int or int*?
			case 'l': ::run<sandpile::_array_stack<int>, sandpile::_fix_log_l<int>>(grid, chips, hint); break;
			case 's':
				::run<sandpile::_array_stack<int>, sandpile::_fix_log_s<int>>(grid, chips, hint);
				std::cout << grid;
				break;
		}

		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Runs the rotor router algorithm until all chips are out.";
	help.input = "arrow grid";
	help.syntax = "rotor/rotor s|l <shell command> [<hint>]";
	help.add_param("s|l", "s calculates resulting arrows, l the number each arrow fires");
	help.add_param("<shell command>", "calculates chip configuration to add");
	help.add_param("<hint>", "only ensures that arrow at hint will be fired");

	MyProgram program;
	return program.run(argc, argv, &help);
}
