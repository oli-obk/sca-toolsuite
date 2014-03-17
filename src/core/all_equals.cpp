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
#include <cassert>

#include "general.h"
#include "io.h"

class MyProgram : public Program
{
	int main()
	{
		std::vector<int> grid;
		dimension dim;
		int expected = 0;

		if(argc==2)
		 expected = atoi(argv[1]);
		else
		 exit_usage();

		read_grid(stdin, &grid, &dim);

		unsigned int i=0;
		for(; i<dim.area_without_border() && grid[human2internal(i,dim.width)]==expected; i++);

		return (i!=dim.area_without_border());
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Checks whether all cells in a grid equal given value";
	help.input = "grid to test";
	help.syntax = "core/all_equals [<value>]";
	help.add_param("value", "value to test for, defaults to 0");
	help.return_value = "0 iff fulfilled";

	MyProgram program;
	return program.run(argc, argv, &help);
}
