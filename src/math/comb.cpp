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
#include "geometry.h"

inline int add_ints(const int i1, const int i2) { return i1+i2; }
inline int sub_ints(const int i1, const int i2) { return i1-i2; }
inline int mul_ints(const int i1, const int i2) { return i1*i2; }
inline int max_of_ints(const int i1, const int i2) { return std::max(i1, i2); }
inline int min_of_ints(const int i1, const int i2) { return std::min(i1, i2); }

class MyProgram : public Program
{
	int main()
	{
		if(argc != 3)
		 exit_usage();

		const char* op_str = argv[1];

		int (*op_fptr)(const int i1, const int i2) = NULL;
		if(!strcmp(op_str,"add")) op_fptr = &add_ints;
		else if(!strcmp(op_str,"sub")) op_fptr = &sub_ints;
		else if(!strcmp(op_str,"mul")) op_fptr = &mul_ints;
		else if(!strcmp(op_str,"max")) op_fptr = &max_of_ints;
		else if(!strcmp(op_str,"min")) op_fptr = &min_of_ints;
		else exitf("Unknown operator %s. Supported: add, sub, mul, max, min.\n", op_str);

		std::vector<int> grid1, grid2, grid0;
		dimension dim, dim2;
		read_grid(stdin, &grid1, &dim);

		get_input(argv[2]);
		read_grid(stdin, &grid2, &dim2);

		if(dim != dim2)
		 exit("Different dimensions in both grids are not allowed.");

		grid0.resize(dim.area());
		register unsigned int idx;
		for(unsigned int i=0; i<dim.area_without_border(); i++)
		{
			idx = human2internal(i,dim.width());
			grid0[idx] = op_fptr(grid1[idx], grid2[idx]);
		}

		write_grid(stdout, &grid0, &dim);
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Combines two input grids to one.";
	help.syntax = "math/comb add|sub|min|max <shell command>";
	help.input = "first input grid";
	help.add_param("<shell command>", "Command to generate second input grid. Double quotes suggested.");

	MyProgram p;
	return p.run(argc, argv, &help);
}
