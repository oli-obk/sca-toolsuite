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

#ifndef CA_H
#define CA_H

/**
 * @brief This class holds anything a cellular automaton's function
 * needs to know.
 *
 * Thus it contains no grid.
 */
class ca_simulator_t
{
	eqsolver::expression_ast ast;
	int* helper_vars = nullptr; //!< @todo: auto_ptr
	int helpers_size;
	int border_width;
public:
	~ca_simulator_t() { delete[] helper_vars; }

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	ca_simulator_t(const char* equation)
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

	int next_state(int *cell_ptr, int x, int y, const dimension& dim)
	{
		// TODO: replace &((*old_grid)[internal]) by old_value
		// and make old_value a ptr/ref?
		eqsolver::variable_print vprinter(dim.height, dim.width,
			x,y,
			cell_ptr, helper_vars);
		eqsolver::ast_print<eqsolver::variable_print> solver(&vprinter);
		return (int)solver(ast);
	}
	int next_state_realxy(int *cell_ptr, int x, int y, const dimension& dim)
	{
		return next_state(cell_ptr, x-border_width, y-border_width, dim);
	}
	int next_state_gridptr_realxy(int *grid_ptr, int x, int y, const dimension& dim)
	{
		const int internal = x+y*dim.width;
		return next_state_realxy(grid_ptr + internal, x, y, dim);
	}
	int next_state_gridptr(int *grid_ptr, int x, int y, const dimension& dim)
	{
		const int internal = (x + border_width) + (y + border_width) * dim.width;
		return next_state(grid_ptr + internal, x, y, dim);
	}

};

#endif // CA_H
