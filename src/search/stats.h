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

#ifndef STATS_H
#define STATS_H

#include <ctime>
#include "types.h"

struct stat_results_t
{ // TODO: extra header?
	std::size_t sorted_out_1, sorted_out_2, total;
	std::size_t runtime_sec;
	friend io::serializer& operator<<(io::serializer& s, const stat_results_t& r)
	{
		return s << r.sorted_out_1 << r.sorted_out_2 << r.total << r.runtime_sec;
	}

	friend io::deserializer& operator>>(io::deserializer& s, stat_results_t& r)
	{
		return s >> r.sorted_out_1 >> r.sorted_out_2 >> r.total >> r.runtime_sec;
	}
};

class stats_t : public types
{
	std::size_t n_verts = 0;
	std::set<point> super_area; //!< records the maximum of all ever activated cells
	std::size_t extra_nodes = 0, extra_stack = 0,
		 movable_nodes = 0, isolated = 0;
	bool has_extra_nodes;
	static constexpr std::size_t dump_interval() { return 1000; }
	int cur_vertex;
	std::size_t tree_depth = 0;
#if 0
	std::size_t runtime_s = 0;
#endif
	clock_t start_time;

	struct depth_wise_t
	{
		std::size_t nodes_started = 0,
			//waiting_nodes = 0,
			est_nodes = 0, // est for next level
			cur_remain = 0, // cur remain for next level
			remained = 0; // total remain for next level
	};
	std::vector<depth_wise_t> at_depth;

	std::size_t get_runtime() const {
		return ((float)clock()-(float)start_time)/CLOCKS_PER_SEC;
	}

public:
	stat_results_t make_stat_results() const
	{
		return stat_results_t { movable_nodes, isolated, n_verts,
			get_runtime() };
	}

	void set_cur_vertex(const int& _cur_vertex) { cur_vertex = _cur_vertex; }
	void inform_new_vertex(const std::set<point>& area)
	{
		for(const point& p : area)
		 super_area.insert(p);
		if(!(++n_verts % dump_interval())) dump();
	}
	void inform_extra_node() { ++extra_nodes; }
	void inform_extra_node_stack() { ++extra_stack; }
	void inform_movable_node() { ++movable_nodes; }
	void inform_isolated_point() { ++isolated; }
	void dump() const;
	stats_t(bool has_extra_nodes)
		: has_extra_nodes(has_extra_nodes),
		start_time(clock()),
		at_depth {1} // one root node (TODO)
		 {}
	~stats_t() {
		if(tree_depth != 0)
		 throw "Error: Tree depth not 0 on exiting.";
		dump();
	}

	void est_cur_children(std::size_t estimation)
	{
		if(at_depth.size() <= (tree_depth + 1)) {
			at_depth.emplace_back();
		}

		at_depth[tree_depth].est_nodes += estimation;
		at_depth[tree_depth].cur_remain = estimation;
	}

	void inc_tree_depth()
	{
		++tree_depth;
		if(at_depth.size() <= tree_depth) {
			at_depth.emplace_back();
		}

		++at_depth[tree_depth].nodes_started;

	//	std::cerr << "++depth: " << tree_depth << std::endl;
	}

	void dec_tree_depth()
	{
		at_depth[tree_depth].remained += at_depth[tree_depth].cur_remain;

		--tree_depth;

		--at_depth[tree_depth].cur_remain;
	//	std::cerr << "--depth: " << tree_depth << std::endl;
	}

};
#endif // STATS_H
