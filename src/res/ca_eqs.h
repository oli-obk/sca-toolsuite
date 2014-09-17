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
#include "bitgrid.h"

namespace sca { namespace ca {

//#define CA_DEBUG

class eqsolver_t // TODO: can we move the private members to inheriting class?
{
private:
	eqsolver::expression_ast ast;
	int* helper_vars = nullptr; //!< @todo: auto/unique_ptr
	std::size_t helpers_size;
	std::size_t _num_states;
//	n_t_const neighbourhood;

public:
	std::size_t num_states() const noexcept { return _num_states; }

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

	~eqsolver_t() noexcept { delete[] helper_vars; }

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	eqsolver_t(const char* equation, unsigned num_states = 0) // TODO: cpp file
		: _num_states(num_states)
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


	template<class Src, class Tar, class T>
	int calculate_next_state(const Src& src_array, const Tar& tar_array,
		const _point<T>& p) const
	{
		// TODO: replace &((*old_grid)[internal]) by old_value
		// and make old_value a ptr/ref?
		using vprinter_t = eqsolver::_variable_print<Src, Tar>;
		vprinter_t vprinter(
			p.x, p.y,
			src_array, tar_array, helper_vars);
		eqsolver::ast_print<vprinter_t> solver(&vprinter);
		return (int)solver(ast);
	}

	template<class T, class CT>
	int calculate_next_state_old(const typename CT::cell_t *cell_ptr,
		const _point<T>& p, const _dimension<T>& dim) const
	{
		int result; // TODO: used?
		return calculate_next_state(
			eqsolver::const_grid_storage_array(cell_ptr, dim.width()),
			eqsolver::grid_storage_single(&result),
			p
			);
	}

	//! version for multi-targets
	// TODO: point should be 1st arg, dim 2nd
	// TODO: choose useful return value
	template<class T, class CT>
	int calculate_next_state(const typename CT::cell_t *cell_ptr,
		const _point<T>& p, const _dimension<T>& dim, typename CT::cell_t *cell_tar,
		const _dimension<T>& tar_dim) const
	{
		return calculate_next_state(
			eqsolver::const_grid_storage_array(cell_ptr, dim.width()),
			eqsolver::grid_storage_array(cell_tar, tar_dim.width()),
			p
			);
	}


	//! Runtime: depends on formula.
	// TODO: bit storage grids?
	template<class T, class = void>
	int calculate_next_state(uint64_t grid_int, uint64_t size_each,
		const _point<T>& p, const _dimension<T>& dim) const
	{
		int eval_idx = dim.width() * p.y + p.x; // TODO: bw?
		int _result; // TODO: used?
		return calculate_next_state(
			eqsolver::grid_storage_bits(grid_int, size_each, dim.width(), eval_idx),
			eqsolver::grid_storage_single(&_result),
			p
			);
	}

	//! Runtime: depends on formula.
	// TODO: bit storage grids?
	template<class T, class CT>
	void calculate_next_state_grids(uint64_t grid_int, uint64_t size_each,
		const _point<T>& p, const _dimension<T>& dim,
		CT* cell_res, const ::dimension& dim_tar) const
	{
		int eval_idx = dim.width() * p.y + p.x; // TODO: bw?
		calculate_next_state(
			eqsolver::grid_storage_bits(grid_int, size_each, dim.width(), eval_idx),
			eqsolver::grid_storage_array(cell_res, dim_tar.width()),
			p
			);
		//return cell_res;
	}
};

}}

#endif // CA_EQS_H
