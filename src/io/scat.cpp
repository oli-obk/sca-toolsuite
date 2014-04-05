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

class MyProgram : public Program
{
	int main()
	{
		std::ifstream ifs;
		std::istream *is_ptr;

		switch(argc)
		{
			case 2:
				//read_fp = fopen(argv[1], "r");
				ifs.open(argv[1]);
				if(!ifs.good())
				 exit("Error opening infile");
				is_ptr = &ifs;
				break;
			case 1:
				is_ptr = &std::cin;
				break;
			default:
				exit_usage();
		}

		std::cout << grid_t(*is_ptr);
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
