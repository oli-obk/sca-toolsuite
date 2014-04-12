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

#include "general.h"
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
		eqsolver::build_tree(equation, &ast);

		std::vector<int> grid, result;
		dimension dim;
		read_grid(stdin, &grid, &dim);
		result.reserve(grid.size());

		/*for(unsigned int y = 0; y<dim.height-2; y++)
		 for(unsigned int x = 0; x<dim.width-2; x++)
		{
			const int human = x+y*(dim.width-2);
			const int internal = human2internal(human, dim.width);
			if(grid[internal] != INT_MIN) { // excludes border
				printf("REAL	ADDR of v: %d\n",(int)grid.data());
				eqsolver::ast_print solver(dim.height, dim.width, x,y,grid.data());
				printf("%d %d\n",grid[internal], grid[internal]);
				printf("%d %d\n",(grid.data())[internal], (grid.data())[internal]);
				printf("x=%d, y=%d, value=%d, internal: %d => result: %d\n",x,y,grid[internal],internal,solver(ast));
				result[internal] = solver(ast);
				printf("%d %d\n",grid[internal], grid[internal]);
				printf("%d %d\n",(grid.data())[internal], (grid.data())[internal]);
			}
			else
			 result[internal];
		}*/
		for(unsigned int y = 0; y<dim.height; y++)
		for(unsigned int x = 0; x<dim.width; x++)
		{
			const int internal = x+y*dim.width;
			if(grid[internal] == INT_MIN) { // excludes border
				result[internal] = INT_MIN;
			}
			else {
				eqsolver::variable_print vprinter(dim.height, dim.width, x-1 ,y-1, &grid[internal], NULL);
				eqsolver::ast_print<eqsolver::variable_print> solver(&vprinter);
				result[internal] = (int)solver(ast);
			}
		}


		write_grid(stdout, &result, &dim);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "math/equation <equation>";
	help.description = eqsolver::get_help_description();
	help.input = "grid to be modified";
	help.output = "modified grid";
	help.add_param("<equation>", "Manipulation formula in x,y and v(=value). Double quotes suggested.");

	MyProgram p;
	return p.run(argc, argv, &help);
}

