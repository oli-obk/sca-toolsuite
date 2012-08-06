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

#include "equation_solver.h"
#include "io.h"

class MyProgram : public Program
{
	int main()
	{
		int num_steps = INT_MAX;
		eqsolver::expression_ast ast;

		switch(argc)
		{
			case 3:
				num_steps = atoi(argv[2]);
			case 2:
				build_tree_from_equation(argv[1], &ast);
				break;
			case 1:
				break;
			default:
				exit_usage();
		}

		eqsolver::ast_area<eqsolver::variable_area_grid> grid_solver;
		const int border_width = grid_solver(ast);
		debugf("Size of Moore Neighbourhood: %d\n", border_width);

		eqsolver::ast_area<eqsolver::variable_area_helpers> helpers_solver;
		const int helpers_size = helpers_solver(ast) + 1;
		debugf("Size of Helper Variable Array: %d\n", helpers_size);
		int *helper_vars = NULL;
		if(helpers_size > 0)
		 helper_vars = new int[helpers_size];

		int num_changed = 1;
		dimension dim;
		std::vector<int> grid[2];
		std::vector<int> *old_grid = grid, *new_grid = grid;

		read_grid(stdin, old_grid, &dim, &read_number, border_width);
		grid[1] = grid[0]; // fit borders

		for(int round = 1; (round <= num_steps)&&num_changed; ++round)
		{
			num_changed = 0;
			old_grid = grid + (round&1);
			new_grid = grid + ((round+1)&1);

			for(unsigned int y = border_width; y<dim.height-border_width; y++)
			for(unsigned int x = border_width; x<dim.width-border_width; x++)
			{
				const int internal = x+y*dim.width;
				int new_value, old_value = (*old_grid)[internal];

				eqsolver::variable_print vprinter(dim.height, dim.width,
					x-border_width,y-border_width,
					&((*old_grid)[internal]), helper_vars);
				eqsolver::ast_print<eqsolver::variable_print> solver(&vprinter);
				(*new_grid)[internal] = (new_value = solver(ast));
				num_changed += (int)(new_value!=old_value);
			}
		}

		delete helper_vars;
		write_grid(stdout, new_grid, &dim, &write_number, border_width);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "ca/ca <equation> [<rounds>]";
	help.description = ".";
	help.input = "";
	help.output = "";
	help.add_param("infile", "specifies a file to read a grid from");

	MyProgram p;
	return p.run(argc, argv, &help);
}

