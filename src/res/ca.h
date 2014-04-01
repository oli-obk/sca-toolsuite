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

#ifndef CA_H
#define CA_H

#include "ca_basics.h"
#include "equation_solver.h"

// TODO:
#define TABLE_OPTIMIZATION

/**
 * @brief This class holds anything a cellular automaton's function
 * needs to know.
 *
 * Thus it contains no grid.
 */
class ca_calculator_t
{
	eqsolver::expression_ast ast;
	int* helper_vars = nullptr; //!< @todo: auto_ptr
	int helpers_size;
	int border_width;
public:
	~ca_calculator_t() { delete[] helper_vars; }

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	ca_calculator_t(const char* equation)
	{
	//	debug("Building AST from equation...\n");
		build_tree_from_equation(equation, &ast);

		eqsolver::ast_area<eqsolver::variable_area_grid>
			grid_solver;
		border_width = (int)grid_solver(ast);
	//	printf("Size of Moore Neighbourhood: %d\n",
	//		border_width);

		eqsolver::ast_area<eqsolver::variable_area_helpers>
			helpers_solver;
		helpers_size = (int)helpers_solver(ast) + 1;
	//	printf("Size of Helper Variable Array: %d\n",
	//		helpers_size);
		if(helpers_size > 0)
		 helper_vars = new int[helpers_size];
	}
	int get_border_width() { return border_width; }

	//! calculates next state at (human) position (x,y)
	//! @param dim the grids internal dimension
	int next_state(const int *cell_ptr, const point& p, const dimension& dim) const
	{
		// TODO: replace &((*old_grid)[internal]) by old_value
		// and make old_value a ptr/ref?
		eqsolver::variable_print vprinter(dim.height, dim.width,
			p.x, p.y,
			cell_ptr, helper_vars);
		eqsolver::ast_print<eqsolver::variable_print> solver(&vprinter);
		return (int)solver(ast);
	}

	//! overload, with x and y in internal format. slower.
	int next_state_realxy(const int *cell_ptr, const point& p, const dimension& dim) const
	{
		return next_state(cell_ptr, p - point { border_width, border_width }, dim);
	}
	//! overload with human coordinates, but pointer to grid. slower.
	int next_state_gridptr(const int *grid_ptr, const point& p, const dimension& dim) const
	{
		const int internal = (p.x + border_width) + (p.y + border_width) * dim.width;
		return next_state(grid_ptr + internal, p, dim);
	}
	//! overload with x and y in internal format and
	//! grid_ptr pointing to beginning of grid. most slow.
	int next_state_gridptr_realxy(const int *grid_ptr, const point& p, const dimension& dim) const
	{ // TODO: save dim by using grid_t instead of grid_ptr
		const int internal = p.x+p.y*dim.width;
		return next_state_realxy(grid_ptr + internal, p, dim);
	}

	neighbourhood get_neighbourhood() const
	{
		unsigned moore_width = (border_width<<1) + 1;
		dimension moore = { moore_width, moore_width };
		return neighbourhood(moore, point(border_width, border_width));
	}
};

class ca_simulator_t : private ca_calculator_t
{
	grid_t _grid[2];
	grid_t *old_grid = _grid, *new_grid = _grid;
	neighbourhood neighbours;
	std::vector<point> //recent_active_cells(old_grid->size()),
			new_active_cells; // TODO: this vector will shrink :/
	std::set<point> cells_to_check; // TODO: use pointers here, like in grid
	int round = 0;
	bool async;
public:
	ca_simulator_t(const char* equation, bool async = false) :
		ca_calculator_t(equation),
		_grid({get_border_width(), get_border_width()}),
		neighbours(get_neighbourhood()),
		async(async)
	{
	}

	grid_t& grid() { return *new_grid; }
	const grid_t& grid() const { return *new_grid; }

	void finalize()
	{
		_grid[1] = _grid[0]; // fit borders

		// make all cells active, but not those close to the border
		// TODO: make this generic for arbitrary neighbourhoods
		new_active_cells.reserve(old_grid->size());
		for( const point &p : old_grid->points() ) {
			new_active_cells.push_back(p); }
	}

	void run_once()
	{
		old_grid = _grid + ((round+1)&1);
		new_grid = _grid + ((round)&1);

		cells_to_check.clear();
		for(const point& ap : new_active_cells)
		for(const point& np : neighbours)
		{
			const point p = ap + np;
			if(!old_grid->point_is_on_border(p))
			 cells_to_check.insert(p);
		}
		new_active_cells.clear();

		for(const point& p : cells_to_check )
		if(!async || get_random_int(2))
		// TODO: use bool async template here to increase speed?
		// plus: exploit code duplication?
		{
			int new_value;
			const int old_value = (*old_grid)[p];

			(*new_grid)[p] = (new_value
				= next_state(&((*old_grid)[p]), p, _grid->dim()));
			if(new_value != old_value)
			{
				new_active_cells.push_back(p);
			}

		}
		else
		{	// i.e. async + not activated
			// we still need to assign the old value:
			(*new_grid)[p] = (*old_grid)[p];
		}

		++round;
	}

	bool can_run() {
		return (new_active_cells.size()||async);
	}
};

#endif // CA_H
