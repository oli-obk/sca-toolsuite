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
#include "stack_algorithm.h"

class MyProgram : public Program
{
	int main()
	{
		FILE* read_fp = stdin;
		std::vector<int> grid;
		dimension dim;

		assert_usage(argc==1);

		read_grid(read_fp, &grid, &dim);
		const unsigned int width_without_border = dim.width - 2;

		// add border grains
		for(unsigned int human = 0; human<dim.area_without_border(); human++)
		{
			const int as_internal = human2internal(human, dim.width);
			if(human%width_without_border == 0 || (human+1)%width_without_border == 0)
			 grid[as_internal]++; // left or right border
			if(human<width_without_border ||
				human >= dim.area_without_border() - width_without_border)
			 grid[as_internal]++; // top or bottom border
		}

		// stabilize everything
		// note: it is not completely trivial that every cell fires at most once
		sandpile::array_queue array(dim.area());
		for(unsigned int human = 0; human<dim.area_without_border(); human++)
		{
			const int as_internal = human2internal(human, dim.width);
			if(grid[as_internal]>3)
			 sandpile::avalanche_1d_hint_noflush(
				&grid, &dim, as_internal, &array, stdout);
		}

		const bool recurrent = array.size() == dim.area_without_border();
		array.flush();
		return recurrent ? 0 : 1;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Runs burning test.";
	help.syntax = "algo/burning_test";
	help.input = "grid to test";
	help.output = "resulting avalanches, binary";

	MyProgram p;
	return p.run(argc, argv, &help);
}
