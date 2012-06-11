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

#include "image.h"
#include "io.h"

class MyProgram : public Program
{
	int main()
	{
		rgb min_color(0,0,0), max_color(255,255,255);
		int min_val=0, max_val=3;
		FILE* write_fp = stdout;
		switch(argc)
		{
			case 6:
				write_fp = fopen(argv[5], "w");
				if(write_fp==NULL)
				 exit("Error opening outfile");
			case 5: max_val = atoi(argv[4]);
			case 4: min_val = atoi(argv[3]);
			case 3: max_color.from_str(argv[2]);
			case 2: min_color.from_str(argv[1]);
			case 1:
			//	read_fp = stdin;
				break;
			default:
				exit_usage();
		}

		std::vector<int> grid;
		dimension dim;

		read_grid(stdin, &grid, &dim);

		ColorTable color_table(min_color, max_color, min_val, max_val);
		print_to_tga(write_fp, color_table, grid, dim);

		if(argc==7)
		 fclose(write_fp);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "io/grid2tga [<infile>]";
	help.description = "TODO";
	help.input = "input grid, or none if a file was given as an argument";
	help.output = "the same grid";
	help.add_param("infile", "specifies a file to read a grid from");

	MyProgram p;
	return p.run(argc, argv, &help);
}


