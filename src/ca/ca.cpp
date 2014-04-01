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
		unsigned int seed;
		sim_type sim = sim_type::end;
		FILE* const in_fp = stdin;
		FILE* const out_fp = stdout;

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
				equation = argv[1];
				break;
			case 1:
			default:
				exit_usage();
		}

		if(argc < 6)
		 seed = find_good_random_seed();
		set_random_seed(seed);

		ca_simulator_t simulator(equation);

//		int num_changed = 1;
	/*	dimension dim;
		std::vector<int> grid[2];
		std::vector<int> *old_grid = grid, *new_grid = grid;*/

		const int border_width = simulator.get_border_width();

		//read_grid(stdin, old_grid, &dim, border_width);
		grid_t grid[2] { border_width, border_width };
		grid_t *old_grid = grid, *new_grid = grid;
		old_grid->read(in_fp);
		grid[1] = grid[0]; // fit borders
		const dimension dim = old_grid->dim();

#ifdef TABLE_OPTIMIZATION

#endif

		switch(sim)
		{
			case sim_type::role:
				break;
			case sim_type::anim:
				/*os_clear();*/ break;
			case sim_type::more:
				exit("Sorry, `more' is not supported yet.");
			default:
				break;
		}

		neighbourhood neighbours = simulator.get_neighbourhood();

		std::vector<point> //recent_active_cells(old_grid->size()),
			new_active_cells(old_grid->size()); // TODO: this vector will shrink :/
		std::set<point> cells_to_check; // TODO: use pointers here, like in grid
		// make all cells active, but not those close to the border
		// TODO: make this generic for arbitrary neighbourhoods
		for( point &p : old_grid->points() ) {
			new_active_cells.push_back(p); }

		for(int round = 0; (round < num_steps) && (new_active_cells.size()||async); ++round)
		{
	//		num_changed = 0;
			old_grid = grid + ((round+1)&1);
			new_grid = grid + ((round)&1);

			cells_to_check.clear();
			for(const point& ap : new_active_cells)
			for(const point& np : neighbours)
			{
				const point p = ap + np;
			//	std::cout << "np: " << np << std::endl;
				if(!old_grid->point_is_on_border(p))
				 cells_to_check.insert(p);
			}
			new_active_cells.clear();

			if(sim != sim_type::end)
			{
				if(sim == sim_type::anim)
				 os_clear();
				//write_grid(stdout, old_grid, &dim, border_width);
				grid->write(out_fp);
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

			for(const point& p : cells_to_check )
			if(!async || get_random_int(2))
			// TODO: use bool async template here to increase speed?
			// plus: exploit code duplication?
			{
			/*	const int internal = (p.x + border_width)
					+ (p.y + border_width) * dim.width;*/

	//			std::cout << p << std::endl;

				int new_value;
				//const int old_value = (*old_grid)[internal];
				const int old_value = (*old_grid)[p];

/*				(*new_grid)[internal] = (new_value
					= simulator.next_state(&((*old_grid)[internal]), x, y, dim));
				num_changed += (int)(new_value!=old_value);*/
				(*new_grid)[p] = (new_value
					= simulator.next_state(&((*old_grid)[p]), p, dim));
				if(new_value != old_value)
				{
					new_active_cells.push_back(p);
//					num_changed++; // TODO: remove this
				}

			}
			else
			{	// i.e. async + not activated
				// we still need to assign the old value:
			//	int internal = (x + border_width) + (y + border_width) * dim.width;
				(*new_grid)[p] = (*old_grid)[p];
			}

// unnecessary test:
#if 0
			{
				grid_t changes(0, old_grid->human_dim(), 0);
				for(const point& p : new_active_cells) {
				for(const point& p2 : new_active_cells)
					if(p!=p2)
					{
						point diff = p-p2;
						if(!(diff.x>1||diff.x<-1) ||
							(diff.y>1||diff.y<-1))
							std::cout << "close:" << p << p2 << std::endl;

						assert((diff.x>1||diff.x<-1) ||
							(diff.y>1||diff.y<-1));
					}
				}
				puts("");
				changes.write(out_fp);
				puts("");
			}
#endif

// deprecated:
#if 0
			for(unsigned int y = 0; y<dim.height - (border_width<<1); ++y)
			for(unsigned int x = 0; x<dim.width - (border_width<<1); ++x)
			if(!async || get_random_int(2))
			// TODO: use bool async template here to increase speed?
			// plus: exploit code duplication?
			{
				const int internal = (x + border_width)
					+ (y + border_width) * dim.width;
				int new_value;
				const int old_value = (*old_grid)[internal];

/*				(*new_grid)[internal] = (new_value
					= simulator.next_state(&((*old_grid)[internal]), x, y, dim));
				num_changed += (int)(new_value!=old_value);*/
				new_value = simulator.next_state(&((*old_grid)[internal]), x, y, dim);
				if(new_value != old_value)
				{
					new_active_cells.push_back();
					num_changed++; // TODO: remove this
				}

			}
			else
			{	// i.e. async + not activated
				// we still need to assign the old value:
				int internal = (x + border_width) + (y + border_width) * dim.width;
				(*new_grid)[internal] = (*old_grid)[internal];
			}
#endif
		}

		if(sim == sim_type::anim)
		 os_clear();
		new_grid->write(out_fp);
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
	help.syntax = "ca/ca <equation> "
		"[<sim_type> [<rounds> [sync|async [seed]]]]";
	help.description = "Runs a cellular automaton (ca).";
	help.input = "start configuration of the ca";
	help.output = "configuration after the simulation";
	help.add_param("equation", "specifies the equation which determines the ca");
	help.add_param("rounds", "number of rounds to simulate; if not given, simulates until stable");

	MyProgram p;
	return p.run(argc, argv, &help);
}

