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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>

#include "general.h"
#include "io.h"
#include "stack_algorithm.h"

class MyProgram : public Program
{
	// Note: don't put the parameters into this class. It misses the const otherwise,
	// and thus makes the algorithm a lot slower (tested)
	template<class AvalancheContainer>
	void start(std::vector<int>& grid,
		const dimension& dim,
		const std::vector<int>& random_seq)
	{
		AvalancheContainer avalanche_container(dim.area_without_border());
		FILE* const out_fp = stdout;
		for(unsigned int round = 0; round < random_seq.size(); round++)
		{
			grid[random_seq[round]]++;
			l_hint(&grid, &dim, random_seq[round], &avalanche_container, out_fp);
		}
	}

	int main()
	{
		std::vector<int> grid;
		dimension dim;
		std::vector<int> random_seq;

		assert_usage(argc>=4 && argc <=5);

		if(!strcmp(argv[1], "random"))
		{ // user gives us the random seed, the number, and the initial board via stdin
			read_grid(stdin, &grid, &dim);

			random_seq.resize(atoi(argv[2]));
			set_random_seed(atoi(argv[3]));

			const int area = dim.area_without_border();

			for(std::vector<int>::iterator itr = random_seq.begin();
				itr != random_seq.end(); itr++) {
				*itr = human2internal(get_random_int(area-1), dim.width);
			}
		}
		else if(!strcmp(argv[1], "input"))
		{ // user lets us read "random" sequence from stdin, we create an empty board of wxh
			dim.width = atoi(argv[2]) + 2;
			dim.height = atoi(argv[3]) + 2;
			create_empty_grid(&grid, &dim);

			bool eof = false;
			do
			{
				int i;
				if( fscanf(stdin, "%d", &i) != 1)
				 eof=true;
				else
				 random_seq.push_back(human2internal(i, dim.width));
			} while(!eof);
		}
		else
		 exit_usage();

		bool log_avalanches = false;
		if(argc==5)
		{
			if(!strcmp(argv[4], "l")) log_avalanches = true;
			else if(!strcmp(argv[4], "s")) log_avalanches = false;
			else exit_usage();
		}

		if(log_avalanches) {
			start<ArrayQueue>(grid, dim, random_seq);
		} else {
			start<ArrayStack>(grid, dim, random_seq);
			write_grid(stdout, &grid, &dim);
		}

		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "The random sandpile algorithm, which stabilizes configurations given by randomly thrown grains.\n"
		"There are two modes 'random' and 'input'' with different parameters and behaviour.\n"
		"In 'random', the given input grid is added random numbers.\n"
		"In 'input', the zero grid is added the numbers from the given random sequence";
	help.input = "the initial configuration ('input') or the sequence of numbers ('random').";
	help.syntax = "algo/random_throw random <number> <seed> [<logtype>]\n"
		"algo/random_throw input <width> <height> [<logtype>]";
	help.add_param("(1st parameter)", "defines which of the two modes to use");
	help.add_param("<number>", "number of random numbers to generate");
	help.add_param("<seed>", "random seed for pseudo random number generator");
	help.add_param("<logtype>", "'s' calculates resulting arrows, 'l' the number each arrow fires");

	MyProgram program;
	return program.run(argc, argv, &help);
}

