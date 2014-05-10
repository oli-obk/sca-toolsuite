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
		assert_usage(argc==1);

		std::istream& read_fp = std::cin;
		grid_t grid(read_fp, 1);
		const dimension& hdim = grid.human_dim();
		const unsigned int width = hdim.width();

		// TODO: -> asm.h
		// add border grains
		const coord_t lowest = hdim.height() - 1;
		for(const point& p : hdim.points(0))
		{
			grid[p] +=
				(cell_t)((p.x % width)*((p.x+1)%(width)) == 0) +
				(cell_t)((p.y == 0) || (p.y == lowest));
		}

		// stabilize everything
		// note: it is not completely trivial that every cell fires at most once
		sandpile::array_queue array(hdim.area(), stdout);
		for(const point& p : hdim.points(0))
		{
			if(grid[p] > 3)
			{
				sandpile::avalanche_1d_hint_noflush_single(
					grid, p, array);
			}
		}

		const bool recurrent = array.size() == hdim.area();
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
