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
#include "asm_basic.h"
#include "io.h"

class MyProgram : public Program
{
	int main()
	{
		assert_usage(argc == 1);

		std::vector<int> grid;
		dimension dim;

		read_grid(stdin, &grid, &dim);
		superstabilize(&grid, &dim);
		write_grid(stdout, &grid, &dim);

		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "math/super";
	help.description = "Creates the superstabilization of a given configuration.";
	help.input = "any configuration with values >= 0";
	help.output = "the superstabilization";

	MyProgram p;
	return p.run(argc, argv, &help);
}
