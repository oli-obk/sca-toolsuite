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
		FILE* read_fp = stdin;
		const char* equation = "";
		char separator = ' ';
		switch(argc)
		{
			case 3: assert_usage(!strcmp(argv[2],"newlines"));
				separator = '\n';
			case 2: equation = argv[1]; break;
			default: exit_usage();
		}
		eqsolver::expression_ast ast;
		build_tree_from_equation(equation, &ast);

		int symbols_read;
		int index;

		do {
			symbols_read = fscanf(read_fp, "%d", &index);
			if(symbols_read > 0) {
				eqsolver::variable_print vprinter(index);
				eqsolver::ast_print<eqsolver::variable_print> solver(&vprinter); // TODO: don't set x every time
				fprintf(stdout, "%d%c", (int)solver(ast),separator);
			}
		} while(symbols_read > 0);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "math/calc <equation> [newlines]";
	help.description = get_help_description();
	help.input = "sequence to be modified";
	help.output = "modified sequence";
	help.add_param("<equation>", "Manipulation formula in x. Double quotes suggested.");
	help.add_param("newlines", "newlines are chosen as separators, instead of spaces");

	MyProgram p;
	return p.run(argc, argv, &help);
}
