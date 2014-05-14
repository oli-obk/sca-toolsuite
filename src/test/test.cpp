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
#include "ca.h"

class MyProgram : public Program
{
	int main()
	{
		/*const char* fname = nullptr;
		switch(argc)
		{
			case 2:
				fname = argv[1];
			case 1:
				break;
			default:
				exit_usage();
		}*/
		assert(argc == 2);
	/*	sca::ca::bitgrid_t grid(2, dimension(4, 2), 1, 2, 1);
		std::cout << grid;

		for(const point p : grid.points())
		 std::cout << p << ": " << grid[p] << std::endl;*/

		sca::ca::_ca_calculator_t<sca::ca::ca_table_t> ca(argv[1], 3);
		grid_t grid(std::cin, 0);

		std::cout << ca.next_state(grid, point(1,1)) << std::endl;

		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "test/test";
	help.description = "A file only to test internal"
		"(often new) features.";

	MyProgram p;
	return p.run(argc, argv, &help);
}

