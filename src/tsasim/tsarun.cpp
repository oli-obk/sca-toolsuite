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
//#include "stack_algorithm.h"

class Cell
{
public:
	int num_grains;
	int scc_comp_id;
};

class Grid
{
public: // TODO
	std::vector<Cell> grid;
	dimension dim;

public:
	void create_sccs() {
		assert("TO BE DONE");
	}

	void read_from_vector(const std::vector<int>& _grid, dimension& _dim)
	{
		dim = _dim;
		const unsigned int size = _grid.size();
		grid.resize(size);
		for(unsigned int i=0; i < size; i++)
		{
			grid[i].num_grains = _grid[i];
			grid[i].scc_comp_id = i;
		}
		create_sccs();
	}

	void write_to_vector(std::vector<int>& _grid)
	{
		const unsigned int size = grid.size();
		_grid.resize(size);
		for(unsigned int i=0; i < size; i++)
		 _grid[i] = grid[i].num_grains;
	}

	Grid(dimension& dim)
	{
		const unsigned int size = dim.area();
		grid.resize(size);
		for(unsigned int i=0; i < size; i++)
		{
			grid[i].num_grains = 0;
			grid[i].scc_comp_id = i;
		}
	}

	int throw_grain(int position)
	{
		grid[position].num_grains++;
		// no scc got smaller by this
		// but sccs might have gotten larger

		return 0; // TODO
	}
};


int main(int argc, char** argv)
{
	std::vector<int> _grid;
	dimension dim;
	std::vector<int> random_seq;

	if(argc<4 || argc >5)
	{
		fputs("Error: usage must be of:\n", stderr);
		fprintf(stderr, " * %s random <number> <seed> [logtype](use random(), board via stdin)\n", argv[0]);
		fprintf(stderr, " * %s input <width> <height> [logtype](board 0, random array via stdin)\n", argv[0]);
		fprintf(stderr,"...where logtype must be of `l' `s'(=default).\n");
		exit(1);
	}
	if(!strcmp(argv[1], "random"))
	{ // user gives us the random seed, the number, and the initial board via stdin
		read_grid(stdin, &_grid, &dim);

		random_seq.resize(atoi(argv[2]));
		set_random_seed(atoi(argv[3]));

		const int area = dim.area_without_border();

		for(std::vector<int>::iterator itr = random_seq.begin();
			itr != random_seq.end(); itr++) {
			*itr = human2internal(get_random_int(area-1), dim.width);
		}
	}
	else
	{ // user lets us read "random" sequence from stdin, we create an empty board of wxh
		dim.width = atoi(argv[2]);
		dim.height = atoi(argv[3]);
		_grid.resize(dim.area());

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

	bool log_avalanches = false;
	if(argc==5)
	 log_avalanches = (!strcmp(argv[4], "l"));

	if(log_avalanches) {
		//run<ArrayQueue>(grid, dim, random_seq);
	} else {
		//run<ArrayStack>(grid, dim, random_seq);
		//write_grid(stdout, &grid, &dim);
	}

	Grid grid(dim);
	//grid.read_from_vector(_grid, dim);
	_grid.clear();

	for(unsigned int i=0; i<random_seq.size(); i++)
	 grid.throw_grain(random_seq[i]);

	grid.write_to_vector(_grid);
	write_grid(stdout, &_grid, &dim);

	return 0;
}
