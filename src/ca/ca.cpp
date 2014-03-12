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

#include "equation_solver.h"
#include "io.h"
#include "ca.h"

#define TABLE_OPTIMIZATION

// TODO: own sim type class, inherit
class MyProgram : public Program
{
	enum class sim_type
	{
		end,
		role,
		more,
		anim,
		undefined
	};

	struct sim_wrapper
	{
		sim_type t;
		const char* str;
	};

	sim_wrapper wraps[4] = // TODO: why is 4 needed?
	{
		{ sim_type::end, "end" },
		{ sim_type::role, "role" },
		{ sim_type::more, "more" },
		{ sim_type::anim, "anim" }
	};

	sim_type type_by_str(const char* str)
	{
		sim_type sim = sim_type::undefined;
		for(sim_wrapper& i : wraps)
		 if(!strcmp(i.str, str))
		  sim = i.t;
		return sim;
	}

	int main()
	{
		const char* equation;
		bool async = false;
		int num_steps = INT_MAX;
		eqsolver::expression_ast ast;
		unsigned int seed;
		sim_type sim = sim_type::end;

		switch(argc)
		{
			case 6:
				seed = atoi(argv[5]);
			case 5:
				assert_usage(!strcmp(argv[4],"async")
					|| !strcmp(argv[4],"sync"));
				async = (argv[4][0] == 'a');
			case 4:
				num_steps = atoi(argv[3]);
			case 3:
				sim = type_by_str(argv[2]);
				if(sim == sim_type::undefined)
				 exit_usage();
			case 2:
				debug("Building AST from equation...\n");
				build_tree_from_equation(argv[1], &ast);
				equation = argv[1];
				break;
			case 1:
			default:
				exit_usage();
		}

		if(argc < 6)
		 seed = find_good_random_seed();
		set_random_seed(seed);

		equation = argv[1];
		ca_simulator_t simulator(equation);

		int num_changed = 1;
		dimension dim;
		std::vector<int> grid[2];
		std::vector<int> *old_grid = grid, *new_grid = grid;

		const int border_width = simulator.get_border_width();
		read_grid(stdin, old_grid, &dim, border_width);
		grid[1] = grid[0]; // fit borders

#ifdef TABLE_OPTIMIZATION

#endif

		switch(sim)
		{
			case sim_type::role:
				break;
			case sim_type::anim:
				os_clear(); break;
			case sim_type::more:
				exit("Sorry, `more' is not supported yet.");
			default:
				break;
		}

		for(int round = 0; (round < num_steps) && (num_changed||async); ++round)
		{
			num_changed = 0;
			old_grid = grid + ((round+1)&1);
			new_grid = grid + ((round)&1);

			if(sim != sim_type::end)
			{
				if(sim == sim_type::anim)
				 os_clear();
				write_grid(stdout, old_grid, &dim, border_width);
				puts("");
				switch(sim)
				{
					case sim_type::anim:
						os_sleep(1);
						break;
					case sim_type::more:
						while(getchar()!='\n') ;
						break;
					default: break;
				}
			}

			for(unsigned int y = 0; y<dim.height - (border_width<<1); ++y)
			for(unsigned int x = 0; x<dim.width - (border_width<<1); ++x)
			if(!async || get_random_int(1))
			// TODO: use bool async template here to increase speed?
			// plus: exploit code duplication?
			{
				const int internal = (x + border_width)
					+ (y + border_width) * dim.width;
				int new_value;
				const int old_value = (*old_grid)[internal];

				(*new_grid)[internal] = (new_value
					= simulator.next_state(&((*old_grid)[internal]), x, y, dim));
				num_changed += (int)(new_value!=old_value);
			}
			else
			{	// i.e. async + not activated
				// we still need to assign the old value:
				int internal = (x + border_width) + (y + border_width) * dim.width;
				(*new_grid)[internal] = (*old_grid)[internal];
			}
		}

		write_grid(stdout, new_grid, &dim, border_width);
		if(sim == sim_type::anim) {
			puts("");
			os_sleep(1);
		}

		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "ca/ca <equation>"
		"[<sim_type> [<rounds> [sync|async [seed]]]]";
	help.description = "Runs a cellular automaton (ca).";
	help.input = "start configuration of the ca";
	help.output = "configuration after the simulation";
	help.add_param("equation", "specifies the equation which determines the ca");
	help.add_param("rounds", "number of rounds to simulate; if not given, simulates until stable");

	MyProgram p;
	return p.run(argc, argv, &help);
}

