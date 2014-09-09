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

#ifndef CA_EQS_H
#define CA_EQS_H

#include <map>
#include <stack>
#if 0
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#endif

#include "ca_basics.h"
#include "equation_solver.h"

namespace sca { namespace ca {

//#define CA_DEBUG

class eqsolver_t // TODO: can we move the private members to inheriting class?
{
private:
	eqsolver::expression_ast ast;
	int* helper_vars = nullptr; //!< @todo: auto/unique_ptr
	std::size_t helpers_size;
protected:
	unsigned num_states;
//	n_t_const neighbourhood;

	template<class Traits>
	typename Traits::u_coord_t calc_border_width() const
	{
		eqsolver::ast_area<eqsolver::variable_area_grid>
			grid_solver;
		return (int)grid_solver(ast);
	}

	template<class Traits>
	_n_t<Traits,std::vector<_point<Traits>>> calc_n_in() const
	{
		using point = _point<Traits>;
		eqsolver::ast_area_cont<eqsolver::variable_area_cont<std::set<point>>>
			grid_solver_2(true);
		std::set<point> res = grid_solver_2(ast);
		std::vector<point> res_v;
		std::move(res.begin(), res.end(), std::back_inserter(res_v));
		return _n_t<Traits,std::vector<point>>(std::move(res_v));
	}

	template<class Traits>
	_n_t<Traits,std::vector<_point<Traits>>> calc_n_out() const
	{
		using point = _point<Traits>;
		eqsolver::ast_area_cont<eqsolver::variable_area_cont<std::set<point>>>
			grid_solver_2(false);
		std::set<point> res = grid_solver_2(ast);
		std::vector<point> res_v;
		std::move(res.begin(), res.end(), std::back_inserter(res_v));
		return _n_t<Traits,std::vector<point>>(std::move(res_v));
	}

protected:
	~eqsolver_t() noexcept { delete[] helper_vars; }

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	eqsolver_t(const char* equation, unsigned num_states = 0) // TODO: cpp file
		: num_states(num_states)
	{
		//	debug("Building AST from equation...\n");
		eqsolver::build_tree(equation, &ast);

#ifdef CA_DEBUG
		std::cout << "Input neighbourhood: " << _n_in << std::endl;
		std::cout << "Output neighbourhood: " << _n_out << std::endl;
#endif

#ifdef CA_DEBUG
		printf("Size of Moore Neighbourhood: %d\n", // TODO: use cout
		       _border_width);
#endif
		eqsolver::ast_area<eqsolver::variable_area_helpers>
				helpers_solver;
		helpers_size = (int)helpers_solver(ast) + 1;
#ifdef CA_DEBUG
		printf("Size of Helper Variable Array: %d\n",
		       helpers_size);
#endif
		if(helpers_size > 0)
			helper_vars = new int[helpers_size];

#if 0
		eqsolver::ast_minmax minmax_solver(helpers_size);
		//std::pair<int, int> mm = (std::pair<int, int>)minmax_solver(ast);
		/*	num_states = (mm.first == INT_MIN || mm.second == INT_MAX)
	? INT_MAX
	: (mm.second - mm.first + 1);*/
		std::pair<eqsolver::expression_ast, eqsolver::expression_ast> mm
				= (std::pair<eqsolver::expression_ast, eqsolver::expression_ast>)minmax_solver(ast);

		eqsolver::ast_dump dumper;
		std::cout << "original: " << dumper(ast) << std::endl;

		std::cout << "mm first: " << (std::string)dumper(mm.first) << std::endl; // TODO: non return syntax
		(void)mm;
#endif
		//num_states = 0;
	}

	template<class T, class CT>
	int calculate_next_state(const typename CT::cell_t *cell_ptr,
		const _point<T>& p, const _dimension<T>& dim) const
	{
		eqsolver::const_grid_storage_array arr(cell_ptr, dim.width());
		int result;
		eqsolver::grid_storage_single tar(&result);

		using vprinter_t = eqsolver::_variable_print<>;
		vprinter_t vprinter(
			p.x, p.y,
			arr, tar, helper_vars);
		eqsolver::ast_print<vprinter_t> solver(&vprinter);
		return (int)solver(ast);
	}

	//! version for multi-targets
	// TODO: point should be 1st arg, dim 2nd
	// TODO: choose useful return value
	template<class T, class CT>
	int calculate_next_state(const typename CT::cell_t *cell_ptr,
		const _point<T>& p, const _dimension<T>& dim, typename CT::cell_t *cell_tar,
		const _dimension<T>& tar_dim) const
	{
		// TODO: replace &((*old_grid)[internal]) by old_value
		// and make old_value a ptr/ref?
		// TODO: const cast
		eqsolver::const_grid_storage_array arr(cell_ptr, dim.width());
		eqsolver::grid_storage_array tar(cell_tar, tar_dim.width());

		// TODO: why do we need to specify the default argument?
		using vprinter_t = eqsolver::_variable_print<
			eqsolver::const_grid_storage_array,
			eqsolver::grid_storage_array>;
		vprinter_t vprinter(
			p.x, p.y,
			arr, tar, helper_vars);
		eqsolver::ast_print<vprinter_t> solver(&vprinter);
		return (int)solver(ast);
	}


	//! Runtime: depends on formula.
	// TODO: bit storage grids?
	template<class T, class = void>
		int calculate_next_state(uint64_t grid_int, uint64_t size_each,
		const _point<T>& p, const _dimension<T>& dim) const
	{
		int eval_idx = dim.width() * p.y + p.x; // TODO: bw?
		eqsolver::grid_storage_bits arr(grid_int, size_each, dim.width(), eval_idx);
		int _result;
		eqsolver::grid_storage_single tar(&_result);

		using vprinter_t = eqsolver::_variable_print<eqsolver::grid_storage_bits>;
		vprinter_t vprinter(
			p.x, p.y,
			arr, tar, helper_vars);
		eqsolver::ast_print<vprinter_t> solver(&vprinter);
		return solver(ast);
	}
};

}}

#endif // CA_EQS_H
