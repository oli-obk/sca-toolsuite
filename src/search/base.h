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

#ifndef BASE_H
#define BASE_H

#include <vector>
#include <set>

#include "print.h"
#include "ca_table.h"

#include "types.h"
#include "results.h"
#include "scc_algo.h"
#include "common_macros.h"
#ifdef DEBUG_GRAPH
#include "graph/graph.h"
#endif

// TODO: -> general.h
template<class Grid, class Cont, class Conf>
void _reset_grid_to_conf(Grid& _grid, const Conf& c, const Cont& n)
{
	int i = 0;
	for ( const auto& p : n ) {
		_grid[p] = c[i++];
	}
}

class base : public types
{
protected:
	/*
	 * const data
	 */
	using m_ca_t = ca::_calculator_t<
		ca::table_t, char_traits, char_cell_traits>;
	const m_ca_t ca;
	const typename m_ca_t::n_t& ca_n;
	const typename m_ca_t::n_t ca_n_2;
	const cell_t dead_state; //!< 3 for circuit, 2 for stca

	/*
	 * data
	 */
	grid_t sim_grid, orig_grid, orig_grid_unchanged;
	std::set<point> initial_area_all; //! the cells that were marked negative
	std::vector<conf_t> initial_confs;
	std::set<point> all_points; //! all points used in the tree
	m_graph_t<result_t> res_graph;
	std::string name, rgb32; // meta

	/*
	 * functions
	 */
	void print_results_pretty(rec_rval_base_t &res) {
		res.print_results_pretty(orig_grid);
	}

	/**
		@brief reads a file from io

		the following variables will be initialized:
		sim_grid, orig_grid, comp_grid, orig_comp_grid
		new_vertices (deprecated soon?)
		next_free_id (should...)
		initial_area
		dict
		scc_algo (should...)
		stats (should...)
	*/
	void _parse();

public:
	//! ctor taking the equation as string
	base(const char* equation, cell_t border);

	//! ctor taking an istream to a ca table file
	base(std::istream& is, cell_t border);

	virtual ~base() {}

	void parse();

	//! is being called after the file ... what? TODO
	virtual void init() {}

	//! shall run the algorithm after init
	virtual void run(std::ostream& out) = 0;

	//! shall print out results
	virtual std::ostream& print(std::ostream& stream) const = 0;

	//! removes double cylces etc
	void preprocess_results();

	friend std::ostream& operator<< (std::ostream& stream,
		const base& b);

	static bool global_abort;
};


#endif // BASE_H
