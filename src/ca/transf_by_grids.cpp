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

#include <cstring>
#include "general.h"
#include "io.h"
#include "ca_basics.h" // TODO: -> cmake deps

using namespace sca;

// TODO: own symm type class, inherit
class MyProgram : public Program
{
	enum class symmetry_type
	{
		none,
		rotate,
		rotate_mirror
	};

	struct symm_wrapper
	{
		symmetry_type t;
		const char* str;
	};

	const symm_wrapper wraps[4] = // TODO: why is 4 needed?
	{
		{ symmetry_type::none, "none" },
		{ symmetry_type::rotate, "rotate" },
		{ symmetry_type::rotate_mirror, "rotate+mirror" },
	};

	symmetry_type type_by_str(const char* str) const
	{
		for(const symm_wrapper& i : wraps)
		 if(!strcmp(i.str, str))
		  return i.t;
		exitf("Invalid symmetry type %s", str);
		return symmetry_type::none; // suppress compiler error
	}

	exit_t main()
	{
		//int number_of_states;
		//symmetry_type symm_type = symmetry_type::none;
		/*switch(argc)
		{
			case 3:
				symm_type = type_by_str(argv[2]);
			case 2:
				number_of_states = atoi(argv[1]); break;
			default:
				exit_usage();
		}*/

		assert_usage(argc == 1);

		// read neighbour grid
#ifdef SCA_DEBUG
		std::cerr << "Parsing first grid (neighbourhood)..." << std::endl;
#endif
		const ca::n_t neighbours(stdin);
		const point& center_cell = neighbours.get_center_cell();

		std::vector<ca::trans_t> table;

		// read other grids into table
#ifdef SCA_DEBUG
		std::cerr << "Parsing table..." << std::endl;
#endif

		dimension n_dim = neighbours.get_dim();
		grid_t _in_grid(n_dim, 0);

		while(!feof(stdin))
		{
			grid_t in_grid(std::cin, 0);
			grid_t out_grid(std::cin, 0);
			//assert(out_grid.human_dim().area() == 1);
			assert((2 + out_grid.human_dim().dx())
				== in_grid.human_dim().dx());
			assert((2 + out_grid.human_dim().dy())
				== in_grid.human_dim().dy());

			point ul = *in_grid.points().end();
			dimension rc(ul.x-2, ul.y-2);
			for(const point& p : rc)
			{
				in_grid.copy_to_int(_in_grid, n_dim + p);

				const def_cell_traits::cell_t out_cell
					= out_grid[p];

				neighbours.add_transition_functions(
						table, center_cell, _in_grid, out_cell
					);
			}
		}

		assert(table.size() > 0);

		// uniq assertion
		std::sort(table.begin(), table.end(), ca::compare_by_input);
		const ca::trans_t* recent = &(table[0]);

		for(std::vector<ca::trans_t>::const_iterator itr
			= (++table.begin()); itr != table.end(); ++itr)
		{
			assert(*itr != *recent);
			recent = &*itr;
		}

		// write to out
		std::sort(table.begin(), table.end());
		{
			std::size_t braces = 0;

			// print helper vars
			for(std::size_t i = 0; i < neighbours.size(); i++)
			{
				point p = neighbours[i];
				printf("h[%lu]:=a[%d,%d],\n",
					i, p.x, p.y);
			}

			int recent_output = table[0].get_output();
			puts("(");
			// print functions
			for(const auto& tf : table)
			{
				if(recent_output != tf.get_output())
				{
					printf("0 ) ? %d : (\n"
					"(\n", recent_output);
					++braces;
					recent_output = tf.get_output();
				}

				for(unsigned i = 0; i < neighbours.size(); i++)
				{
					int input_val;
					const bool is_set = tf.input(i, &input_val);
					assert(is_set);
					if(is_set)
					 printf("(h[%d] == %d) && \n",
						i, input_val);
				}
				puts(" 1 ||");
			}
			// keep value as v if no matches
			printf("0 ) ? %d : v", recent_output);
			for(std::size_t i = 0; i < braces; ++i)
			 printf(")");
			puts("");
		}

		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "ca/transf_by_grids";
	//help.syntax = "ca/transf_by_grids <number of states>";
	help.description = "Builds a transition formula by transition grids";
	help.input = "the transition grids, in some special format."
		"The first grid marks the changing cell (0),"
		"the changing cell in the neighborhood (1),"
		"its neighborhoud (2) and dontcares (>2)."
		"Then, there follows a sequence of <input grid> <output grid>"
		"where input grid gives the neighborhood.";
	// TODO: describe?
	help.output = "the formula, as `ca/ca' can read it";
//	help.add_param("number of states", "the ca's number of states");

	MyProgram p;
	return p.run(argc, argv, &help);
}

