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

#include "io.h"

// TODO: own sim type class, inherit

class MyProgram : public Program
{
	class transition_function
	{
		static int neighbour_size;
		std::vector<int> input_vals;
		std::vector<bool> input_set;
		int input_count = 0;
		int output;
		bool initialized = false;
	public:
		transition_function() :
			input_vals(neighbour_size),
			input_set(neighbour_size, false)
		{
		}
		static void set_neighbour_size(int sz) {
			neighbour_size = sz;
		}
		void set_neighbour(int neighbour_id, int val)
		{
			input_vals[neighbour_id] = val;
			if(!input_set[neighbour_id])
			{
				input_set[neighbour_id] = true;
				++input_count;
			}
			else assert(false);
		}
		void set_output(int val) {
			initialized = true;
			output = val;
		}
		int get_output() const { return output; }
		bool input(int neighbour_id, int* result) {
			bool is_set = input_set[neighbour_id];
			if(is_set) *result = input_vals[neighbour_id];
			return is_set;
		}


		//bool is_initialized() const { return initialized; }

		// compares lexicographical, returns results for operator<
		// a dontcare in front counts as a smaller value
		friend bool _compare_by_input(const transition_function& lhs,
			const transition_function& rhs)
		{
			for(int i = 0; i<neighbour_size; ++i)
			{
				if(lhs.input_set[i] || rhs.input_set[i])
				{
					// if one of tem is not set,
					// the one which is not set is smaller
					if(!(lhs.input_set[i]&&rhs.input_set[i]))
					 return (int)lhs.input_set[i]
						> (int)rhs.input_set[i];
					else if(lhs.input_vals[i]
						!=rhs.input_vals[i])
					 return (lhs.input_vals[i]
						<rhs.input_vals[i]);
				}
			}
			return false; // if both are equal
		}

		bool operator<(const transition_function& rhs) const
		{
			return (output == rhs.output)
				? _compare_by_input(*this, rhs)
				: (output < rhs.output);
		}

		bool operator==(const transition_function& rhs) const
		{
			if(input_count != rhs.input_count)
			 return false; // shortening
			for(int i = 0; i<neighbour_size; ++i)
			{
				if(input_set[i]&&rhs.input_set[i])
				{
					if(input_vals[i]!=rhs.input_vals[i])
					 return false;
				}
				else if(input_set[i]!=input_set[i])
				  return false; // one is not set
			}
			return true;
		}

		bool operator!=(const transition_function& rhs) const
		{
			return ! operator==(rhs);
		}
	};

	static bool compare_by_input(const transition_function& lhs,
			const transition_function& rhs)
	{
		return _compare_by_input(lhs, rhs);
	}

	static void set_output_cell(int* output_cell, int value)
	{
		// this forbids to have two output cells
		assert(*output_cell < 0);
		*output_cell = value;
	}

	int main()
	{
		int number_of_states;
		switch(argc)
		{
			case 2:
				number_of_states = atoi(argv[1]); break;
			default:
				exit_usage();
		}

		dimension input_dim;
		std::vector<int> neighbours;
		int output_cell = -1;

		// read neighbour grid
		{
			std::vector<int> neighbour_grid;
			read_grid(stdin, &neighbour_grid, &input_dim, 0);
			for(unsigned y=0, id=0; y<input_dim.height; ++y)
			for(unsigned x=0; x<input_dim.width; ++x, ++id)
			{
				int elem = neighbour_grid[id];
				switch(elem)
				{
				case 0:
					set_output_cell(&output_cell, id);
					break;
				case 1:
					set_output_cell(&output_cell, id);
				case 2:
					neighbours.push_back(id);
					break;
				default: break;
				}
			}
			transition_function::set_neighbour_size(
				neighbours.size());
		}

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
				assert(cur_dim == input_dim);

				read_grid(stdin, &out_grid, &cur_dim, 0);
				assert(cur_dim.height == 1
					&& cur_dim.width == 1);
				out_cell = out_grid[0];
			}

			transition_function new_func;

			for(unsigned i = 0; i < neighbours.size(); ++i)
			 new_func.set_neighbour(i, cur_grid[neighbours[i]]);
			new_func.set_output(out_cell);

			table.push_back(new_func);

			eof = feof(stdin);
		}

		// uniq assertion
		std::sort(table.begin(), table.end(), compare_by_input);
		const transition_function* recent = &(table[0]);
		// TODO: I do not know why refs for recent
		// break the const here...
		for(std::vector<transition_function>::const_iterator itr
			= (++table.begin()); itr != table.end(); ++itr)
		{
			assert(*itr != *recent);
			recent = &(*itr);
		}

		/*		for(auto& tf : table)
		 for(unsigned i = 0; i < neighbours.size(); i++)
		 {
		 int val;
			tf.input(i, &val);
		  printf("input[%d]: %d\n", i, val);
		}*/

		// write to out
		std::sort(table.begin(), table.end());
		{
			int out_x, out_y;
			input_dim.id_to_coords(output_cell, &out_x, &out_y);

			// print helper vars
			for(unsigned i = 0; i < neighbours.size(); i++)
			{
				int rel_x, rel_y;
				input_dim.id_to_coords(neighbours[i],
					&rel_x, &rel_y);
				printf("h[%d]:=a[%d,%d],\n",
					i, rel_x-out_x, rel_y-out_y);
			}

			int recent_output = table[0].get_output();
			puts("(");
			// print functions
			for(auto& tf : table)
			{
				if(recent_output != tf.get_output())
				{
					printf("0 ) ? %d :\n"
					"(\n", recent_output);
					recent_output = tf.get_output();
				}

				for(unsigned i = 0; i < neighbours.size(); i++)
				{
					int input_val;
					bool is_set = tf.input(i, &input_val);
					if(is_set)
					 printf("(h[%d] == %d) && \n",
						i, input_val);
				}
				puts(" 1 ||");
			}
			// keep value as v if no matches
			printf("0 ) ? %d : v\n", recent_output);
		}

		return 0;
	}
};

int MyProgram::transition_function::neighbour_size;

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

