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

#include "io.h"
#include "equation_solver.h"

class MyProgram : public Program
{
	int main()
	{
		const char* equation = "";
		switch(argc)
		{
			case 2: equation = argv[1]; break;
			default: exit_usage();
		}
		eqsolver::expression_ast ast;
		build_tree_from_equation(equation, &ast);

		std::vector<int> grid;
		dimension dim;
		read_grid(stdin, &grid, &dim);

		for(unsigned int y = 0; y<dim.height-2; y++)
		 for(unsigned int x = 0; x<dim.width-2; x++)
		{
			const int human = x+y*(dim.width-2);
			const int internal = human2internal(human, dim.width);
			if(grid[internal] != INT_MIN) { // excludes border
				eqsolver::ast_print solver(x,y,grid[internal]);
				//printf("x=%d, y=%d, value=%d => result: %d\n",x,y,grid[internal],solver(ast));
				grid[internal] = solver(ast);
			}
		}

		write_grid(stdout, &grid, &dim);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "math/equation <equation>";
	help.description = get_help_description();
	help.input = "grid to be modified";
	help.output = "modified grid";
	help.add_param("<equation>", "Manipulation formula in x,y and v(=value). Double quotes suggested.");

	MyProgram p;
	return p.run(argc, argv, &help);
}

