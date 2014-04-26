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
#include "stack_algorithm.h"

class MyProgram : public Program
{
	template<class AvalancheContainer>
	void start(std::vector<int>& grid, const dimension& dim, int hint, int times)
	{
		AvalancheContainer container(dim.area(), stdout);
		if(times < 0)
		 sandpile::l_hint(grid, dim, hint, container);
		else
		 sandpile::lx_hint(grid, dim, hint, container, times);
	}

	int main()
	{
		FILE* read_fp = stdin;
		int hint = -1;
		int times = -1;
		bool avalanches = false;

		switch(argc) {
			case 4: times = atoi(argv[3]);
			case 3: hint = atoi(argv[2]);
			case 2: assert_usage(!argv[1][1] && (argv[1][0]=='l'||argv[1][0]=='s'));
				avalanches = (argv[1][0]=='l');
				break;
			default: exit_usage();
		}

		std::vector<int> grid;
		dimension dim;

		read_grid(read_fp, &grid, &dim);

		if(hint < 0) // TODO: allow cells >= 4 (big for loop...)
		{
			hint = 0; // dummy value
			bool min4_found = false;
			for(unsigned int i=0; i<dim.area_without_border(); i++)
			{
				int cellId = human2internal(i, dim.width());
				if(grid[cellId] > 3)
				{
					if(grid[cellId] > 4)
						exit("For cells with value >= 4, use the program `L'");
					else
					{
						if(min4_found)
						 exit("For grids with 2 cells having value >= 4, use the program `L'");
						else {
							hint = internal2human(cellId, dim.width());
							min4_found = true;
							// continue in order to check for wrong input
						}
					}

				}
			}
		}

		if(avalanches) {
			start<sandpile::array_queue>(grid, dim, human2internal(hint, dim.width()), times);
		} else {
			start<sandpile::array_stack>(grid, dim, human2internal(hint, dim.width()), times);
			write_grid(stdout, &grid, &dim);
		}
		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Runs the stabilisation algorithm until grid is stable.\n"
		"Algorithm runs correct if every cell if only one grain is thrown.\n"
		"More generally, if forcing all cells >= 4 only to fire once leads to\n"
		"no cell firing twice, than the algorithm runs correctly.";
	help.input = "input grid";
	help.syntax = "algo/relax s|l [<hint> [times]]";
	help.add_param("s|l", "s calculates resulting grid, l the number each cell fires");
	help.add_param("<hint>", "only ensures that cell at hint will be fired");
	help.add_param("<times>", "forces cell at <hint> to fire not more than <times> times");

	MyProgram program;
	return program.run(argc, argv, &help);
}

