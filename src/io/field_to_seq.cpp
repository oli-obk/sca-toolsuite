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
		FILE* read_fp=stdin;
		char separator = ' ';

		switch(argc)
		{
			case 2: assert_usage(!strcmp(argv[1],"newlines"));
				separator = '\n';
			case 1: break;
			default: exit_usage();
		}

		std::vector<int> grid;
		dimension dim;

		read_grid(read_fp, &grid, &dim);

		for(unsigned int i=0; i<grid.size(); i++)
		{
			const int human = internal2human(i,dim.width);
			while(grid[i] != INT_MIN && grid[i]--)
			 fprintf(stdout, "%d%c", human, separator);
		}
		fputs("\n", stdout);

		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Computig the unique (until permutation) sequence of numbers\n"
		"which - added - give the specified grid. Separated by spaces.";
	help.input = "input grid";
	help.output = "sequence of numbers";
	help.syntax = "io/field_to_seq [newlines]";
	help.add_param("newlines", "newlines are chosen as separators, instead of spaces");

	MyProgram program;
	return program.run(argc, argv, &help);
}

