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
#include "image.h"
#include "grid.h"

class MyProgram : public Program
{
	int main()
	{
		rgb min_color(255,255,255), max_color(0,0,0);
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
			case 1: break;
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
	help.syntax = "io/grid2tga [<min_color> <max_color> [<min_val> <max_val> [<outfile>]]]";
	help.description = "Converts given grid to a TGA file (v2, color table, no RLE).\n"
		"Colors can be specified for minimum and maximum no of chips,\n"
		"other numbers will be linearly interpolated.";
	help.input = "input grid";
	help.output = "TGA file content if no outfile is given, otherwise nothing";
	help.add_param("<min_color>, <max_color>", "color values for min and max no of chips");
	help.add_param("<min_val>, <max_val>", "modify min and max no of chips - default is 0 and 3");
	help.add_param("<outfile>", "file to store resulting TGA in");
	MyProgram p;
	return p.run(argc, argv, &help);
}


