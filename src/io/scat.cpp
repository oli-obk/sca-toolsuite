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

#include <cstdlib>
#include <cstdio>
#include <vector>
#include "io.h"

class MyProgram : public Program
{
	int main()
	{
		FILE* read_fp;
		switch(argc)
		{
			case 2:
				read_fp = fopen(argv[1], "r");
				if(read_fp==NULL)
				 exit("Error opening infile");
				break;
			case 1:
				read_fp = stdin;
				break;
			default:
				exit_usage();
		}

		std::vector<int> grid;
		dimension dim;

		read_grid(read_fp, &grid, &dim);
		if(argc==2)
		 fclose(read_fp);
		write_grid(stdout, &grid, &dim);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "io/scat [<infile>]";
	help.description = "Reads grid from stdin and writes it to stdout. Useful for formatting.";
	help.input = "input grid, or none if a file was given as an argument";
	help.output = "the same grid";
	help.add_param("infile", "specifies a file to read a grid from");

	MyProgram p;
	return p.run(argc, argv, &help);
}
