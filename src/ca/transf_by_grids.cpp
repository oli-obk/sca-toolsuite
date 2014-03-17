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
#include "ca_basics.h" // TODO: -> cmake deps

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

	symm_wrapper wraps[4] = // TODO: why is 4 needed?
	{
		{ symmetry_type::none, "none" },
		{ symmetry_type::rotate, "rotate" },
		{ symmetry_type::rotate_mirror, "rotate+mirror" },
	};

	symmetry_type type_by_str(const char* str)
	{
		for(symm_wrapper& i : wraps)
		 if(!strcmp(i.str, str))
		  return i.t;
		exitf("Invalid symmetry type %s", str);
		return symmetry_type::none; // suppress compiler error
	}

	int main()
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
		neighbourhood neighbours(stdin);

		std::vector<transition_function> table;

		// read other grids into table
		bool eof = false;
		while(!eof)
		{
			int out_cell;
			std::vector<int> cur_grid;
			{
				std::vector<int> out_grid;
				dimension cur_dim;

				read_grid(stdin, &cur_grid, &cur_dim, 0);
			//	assert(cur_dim == input_dim);

				read_grid(stdin, &out_grid, &cur_dim, 0);
				assert(cur_dim.height == 1
					&& cur_dim.width == 1);
				out_cell = out_grid[0];
			}

			neighbours.add_transition_functions(
				table, cur_grid, out_cell
				);

			eof = feof(stdin);
		}

		assert(table.size() > 0);

		// uniq assertion
		std::sort(table.begin(), table.end(), compare_by_input);
		const transition_function* recent = &(table[0]);
		// TODO: I do not know why const tf&
		// breaks the const here...
		for(std::vector<transition_function>::const_iterator itr
			= (++table.begin()); itr != table.end(); ++itr)
		{
			assert(*itr != *recent);
			recent = &(*itr);
		}

		// write to out
		std::sort(table.begin(), table.end());
		{
			unsigned braces = 0;

			// print helper vars
			for(unsigned i = 0; i < neighbours.size(); i++)
			{
				point p = neighbours[i];
				printf("h[%d]:=a[%d,%d],\n",
					i, p.x, p.y);
			}

			int recent_output = table[0].get_output();
			puts("(");
			// print functions
			for(auto& tf : table)
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
					bool is_set = tf.input(i, &input_val);
					assert(is_set);
					if(is_set)
					 printf("(h[%d] == %d) && \n",
						i, input_val);
				}
				puts(" 1 ||");
			}
			// keep value as v if no matches
			printf("0 ) ? %d : v", recent_output);
			for(unsigned i = 0; i < braces; ++i)
			 printf(")");
			puts("");
		}

		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "ca/transf_by_grids <number of states>";
	help.description = "Builds a transition formula by transition grids";
	help.input = "the transition grids, in some special format."
		"The first grid marks the changing cell (0),"
		"the changing cell in the neighborhood (1),"
		"its neighborhoud (2) and dontcares (>2)."
		"Then, there follows a sequence of <input grid> <output grid>"
		"where input grid gives the neighborhood.";
	// TODO: describe?
	help.output = "the formula, as `ca/ca' can read it";
	help.add_param("number of states", "the ca's number of states");

	MyProgram p;
	return p.run(argc, argv, &help);
}

