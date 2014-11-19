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

#include "print.h"

#include "common_macros.h"
#include "greedy.h"

using namespace brute;

namespace greedy
{

// O(n^3 * not_used_points)
bool algo::has_one_isolated_point(const std::vector<point>& used_points, const std::vector<point>& not_used_points,
	const int src_id) const
{
#define ISOLATED_POINTS
#ifdef ISOLATED_POINTS
	bool has_one_isolated = false;

	const auto scc_areas = sccs_of_area(not_used_points);

	zero_grid[n_out(used_points)] = 1;

	for(const std::vector<point>& p_not_used : scc_areas)
	{
		const auto nu = writers_to(p_not_used);
		auto nu_no_u = nu;
		for(const point& p : p_not_used)
		 nu_no_u.erase(p);

		bool cur_isolated = true;

		/*
		 * condition 1: N(u) \ u must be inactive
		 */
		for(const point& p : nu_no_u)
		if(!bit_equals(recent_grid, sim_grid, p))
		{
			cur_isolated = false;
			break;
		}

		if(!cur_isolated) // => no candidate
		 break;

		auto nnu = writers_to(nu);
		for(const point& p : nu)
		 nnu.erase(p);

		/*
		 * condition 2: no new in-edges from N(N(u)) \ N(u) to N(u)
		 */
		for(const point& p : nnu)
		{
			const auto oe = dep_graph.out_edges(p);

			if(sim_grid.contains(p)) // p can be on the border
			for(auto e_out = oe.begin(); e_out!= oe.end(); ++e_out)
			{
//			std::cerr << "isolated: " << e_out.source() << " -> " << e_out.target() << " (time: " << (*e_out)->time << ")" << std::endl;
			if((dep_graph.get(*e_out).time > src_id) &&
				(nu_no_u.find(e_out.target()) != nu_no_u.end()))
			{
//				std::cerr << " -> not " << std::endl;
				// => found edge from outside N(p)
				cur_isolated = false;
				break;
			}
			}
		}

		if(cur_isolated)
		{
			// this checks: would this conflict with other points?
			for(const point& p : p_not_used)
			for(const point& np : n_out)
			{
				point _p = p + np;
				cur_isolated = cur_isolated && !zero_grid[_p];
				zero_grid[_p] = 1;
			}

			zero_grid[n_out(p_not_used)] = 0;

			if(cur_isolated)
			{
				has_one_isolated = true;
#ifdef VERBOSE_OUTPUT
				std::cerr << "Isolated SCC: " << mk_print(p_not_used) << " at node after " << src_id << std::endl;
#endif
				break;
			}
		}
	}

	zero_grid[n_out(used_points)] = 0;

	return has_one_isolated;
#else
	(void)not_used_points;
	(void)src_id;
	return false;
#endif
}

/**
	@brief TODO
	complexity: O(not_a) * ca
	@param new_area recently activated cells (not: changed cells)
		must be n_out-disjoint
		(they are usually called like this)
	@param bug contains patch of all changed cells (not: activated cells)
	@todo sufficient to pass bug.patch instead of bug
	@todo move this to the header
*/
bool algo::one_scc_of_movables(const std::vector<point>& prev_used,
	const std::set<point>& new_area, const backed_up_grid& bug) const
{
#if 0
	(void)new_area;
	(void)bug;
	return false;
#else
	bool can_move_any_scc = false;
	const auto scc_areas = sccs_of_area(new_area);

	// this applies the old conf on zero_grid
	bug.patch().apply_bwd_force(zero_grid);

	for(const std::vector<point>& v : scc_areas)
	{
		bool can_move_this_scc = true;
		std::size_t set_count = 0;

		// mark non-movables by negating them
		for(const point& p : v)
		if(!bit_equals(prev_grid, sim_grid, p))
		{
			zero_grid[p] = -zero_grid[p];
			++set_count;
		}

		// there is no point to move
		if(set_count == v.size()) {
			can_move_this_scc = false;
			// no need to reset zero_grid
		}
		else for(const point& p : v)
		if(zero_grid[p] < 0) // = not movable
		{
			zero_grid[p] = -zero_grid[p];

			bitgrid_t bits;
			// TODO: get rid of next state call? it's expensive...
			const bool valid = ca.next_state(sim_grid, p, bits);
			if(valid && !bit_equals(bits, zero_grid, p))
			{
#ifdef VERBOSE_OUTPUT
				std::cerr << "point " << (point)p << ": makes unmovable" << std::endl;
#endif
				can_move_this_scc = false;
				break;
			}
			if(!can_move_this_scc)
			 break; // (unclean break)

			// no need to invert back (TODO): this point will not be read again
			zero_grid[p] = -zero_grid[p];
		}

		// prev used points may not conflict with moved ones
		zero_grid_2[n_out(prev_used)] = 1;

		for(const point& p : v)
		if(bit_equals(prev_grid, sim_grid, p)) // = if p is a moved point
		for(const point& np : n_out) {
			can_move_this_scc = can_move_this_scc && zero_grid_2[p+np] == 0;
			zero_grid_2[p+np] = 1; // (TODO:) actually not necessary?
		}

		zero_grid_2[n_out(prev_used)] = 0;
		zero_grid_2[n_out(v)] = 0;

		if(can_move_this_scc)
		{
#ifdef VERBOSE_OUTPUT
			std::cerr << "Movable SCC: " << mk_print(v) << std::endl;
#endif
			can_move_any_scc = true;
			break;
		}
	}

	zero_grid[bug.patch().area()] = 0;

	return can_move_any_scc;
#endif
}

void algo::run(std::ostream& out)
{
	run_base(greedy_detail, out);
}

algo::algo(std::istream &is, cell_t border, bool dump_on_exit) :
	brute_forcer(is, border, dump_on_exit),
	greedy_detail(*this)
{
}

algo::algo(const char *equation, cell_t border, bool dump_on_exit) :
	brute_forcer(equation, border, dump_on_exit),
	greedy_detail(*this)
{
}

}

