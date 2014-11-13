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

#ifndef BRUTE_FORCE_H
#define BRUTE_FORCE_H

#include <boost/bimap.hpp>

#include "stats.h"
#include "disjoint_sets.h"
#include "base.h"

#include "dep_graph.h"

namespace brute
{

struct brute_vertex {
	std::string label;
	bool keep;
	brute_vertex(const char* label, bool keep) : label(label), keep(keep) {}
	brute_vertex() {}
};
struct brute_edge {
	std::string label;
	brute_edge(const char* label) : label(label) {}
	brute_edge() {}
};

#ifdef DEBUG_GRAPH
using debug_graph_t = debug_graph_t<brute_vertex, brute_edge>;
#else
struct debug_graph_t
{
	debug_graph_t(bool ) noexcept {}
	struct vertex_t {};
};
#endif

//! dummy to print out an int as bitgrid, can and should be avoided
struct pseudo_int
{
	uint64_t u;
	explicit operator const uint64_t&() const { return u; }
	explicit operator uint64_t&() { return u; }
	bool operator==(const pseudo_int& other) const {
		return u == other.u;
	}
	bool operator!=(const pseudo_int& other) const {
		return ! operator==(other);
	}
	friend std::ostream& operator<< (std::ostream& stream,
		const pseudo_int& i);
	const pseudo_int& operator+() { return *this; }
};


using bpatch_t = _patch_t<true, char_traits, cell_traits<pseudo_int>>;
using bgrid_t = _grid_t<char_traits, cell_traits<pseudo_int>>;

class _stack_data : types
{
	static int id_counter;
public:
	//! full patch
	const patch_t patch;
	//! used to select new points from power set and
	//! to scan for next active points around
	const std::vector<point> variable;
	//! points that had never been activated before
	const std::vector<point> new_points;
	//! all points that were activated last round
	const std::vector<point> used;
	//! were variable last round, but not activated
	const std::vector<point> not_used;
	//! unique identifier of *this
	const int id;
	//! id of parent in the search tree
	const int src_id;

	//! special case for sort_out_1
	bool parent_edge = false;

	//! patch for recent_grid
	const bpatch_t changable;
	//! debug graph vertex. only for debug graph
	debug_graph_t::vertex_t v_cur;
	//! debug graph source vertex. only for debug graph
	const debug_graph_t::vertex_t v_src;

	result_t* result = nullptr;

	std::vector<std::vector<point>> sccs;
	int next_scc = -1; // TODO: itr?
	uint64_t next_bitmask = 0;

	bool first() const { return next_scc == -1 && next_bitmask == 0; }

	_stack_data(const patch_t&& patch,
		const std::vector<point>&& variable,
		const std::vector<point>&& new_points,
		const std::vector<point>&& used,
		const std::vector<point>&& not_used,
		int src_id,
		const bpatch_t& changable,
		const debug_graph_t::vertex_t& v_src
		) :
			patch(patch),
			variable(variable),
			new_points(new_points),
			used(used),
			not_used(not_used),
			id(++id_counter),
			src_id(src_id),
			changable(changable),
			v_src(v_src)
			{}

	static int next_id() { return id_counter + 1; }
};

class brute_forcer : public base
{
protected:
	/*
	 * protected data
	 */
	m_ca_t::n_t n_in, n_out;
	m_ca_t::n_t readers_of, writers_to;

	using stack_data = _stack_data;
	std::stack<stack_data> stack;

	rec_rval_base_t results; //! @deprecated use base::res_graph instead

	stats_t stats;

	using dict_t = boost::bimap<patch_t, int>;
	dict_t dict;
	scc_algo_t<int> scc_algo;

	grid_scc_finder_t<grid_t> scc_finder;

	//! all active points of recent iteration
	_grid_t<char_traits, cell_traits<pseudo_int>> recent_grid;
	_grid_t<char_traits, cell_traits<pseudo_int>> prev_grid;

	mutable grid_t zero_grid, zero_grid_2; //!< can be used temporary. must be zero outside of use
	// (TODO: use grid_t<bool...>? => measure speedup)

	debug_graph_t debug_graph;

	using m_dep_graph_t = new_dep_graph_t;
	m_dep_graph_t dep_graph, total_dep_graph;

	/*
	 * protected functions
	 */
	bool bit_equals(const bitgrid_t& bitgrid, const grid_t& grid, const point& p) const
	{
		int i = 0;
		const auto is_equal = [&](const point& p2)
		{
			return (cell_t)bitgrid[point(i++, 0)] == grid[p + p2];
		};
		return n_out.for_each_bool(point::zero(), is_equal);
	}

	bool bit_equals(const bgrid_t& _gridgrid, const grid_t& grid, const point& p) const
	{
		const bitgrid_t bitgrid(2/*TODO*/, dimension(n_out.size(), 1),
			0, (uint64_t&)_gridgrid[p]);
		return bit_equals(bitgrid, grid, p);
	}

	void bit_copy(bgrid_t& _gridgrid, const grid_t& grid, const point& p) const
	{
		bitgrid_t bitgrid(2/*TODO*/, dimension(n_out.size(), 1), 0, 0, 0);
		int i = 0;
		const auto copy = [&](const point& p2)
		{
			bitgrid[point(i++, 0)] = grid[p + p2];
		};
		n_out.for_each(point::zero(), copy);
		(uint64_t&)_gridgrid[p] = bitgrid.raw_value();
	}

	bitgrid_t next_state_at(const point& p) const {
		return bitgrid_t(2/*TODO*/, dimension(n_out.size(), 1),
			0, (uint64_t&)recent_grid[p]);
	}

	//! complexity: O(nbh * _area)
	template<class Cont>
	std::vector<std::vector<point>> sccs_of_area(const Cont& _area, const n_t& nbh) const
	{
		zero_grid[_area] = 1;
		const auto cb = [&](const point&, const point& p) {
			return !zero_grid[p]; };
		auto scc_areas = scc_finder(nbh, _area, cb);
		zero_grid[_area] = 0;
		return scc_areas;
	}

	template<class Cont>
	std::vector<std::vector<point>> sccs_of_area(const Cont& _area) const
	{
		return sccs_of_area(_area, readers_of);
	}

	template<class Cont>
	std::vector<types::point> my_variable_points(const Cont &pset) const
	{
		std::vector<point> res;
		for(const point& p : pset)
		for(const auto& np : ca::counted(n_out))
		{
			const point cur = p+(const point&)np;
			// the contains check is annoying... (TODO?)
			if(sim_grid.contains(cur) &&
				(int64_t)next_state_at(p)[point(np.id(), 0)] !=
				sim_grid[cur])
			{
				res.push_back(p);
				break;
			}
		}
		return res;
	}

	//! base for detail of specialized algorithms
	struct detail_default
	{
		std::vector<std::vector<point>> make_sccs(const stack_data& ,
			const std::vector<point>& cur_points) const
		{
			return std::vector<std::vector<point>> {cur_points};
		}

		bool sort_out_1(const std::vector<point>& last_used,
			const std::set<point>& activated, const backed_up_grid& bug) const
		{
			(void)last_used; (void)activated; (void)bug;
			return false;
		}

		bool sort_out_2(const std::vector<point>& used,
			const std::vector<point>& not_used, int src_id) const
		{
			(void)used; (void)not_used; (void)src_id;
			return false;
		}

		void filter(const stack_data&,
			const std::vector<point>& new_vari,
			const std::set<point>& activated) const {
			(void)new_vari; (void)activated;
		}

		void init_node(const stack_data& ) const {}
		void clean_up(const stack_data& ) const {}
		void init_root(const stack_data& ) const {}
	};

	friend struct detail_default;

	//! assuming sim_grid is in the correct conf, inits a new node for it
	//! complexity: O(activated * n * ca_next_state + log(lp))
	template<class Detail>
	void init_new_node(
		const stack_data& cur,
		const patch_t& bug_patch,
		std::set<point>& activated, // exactly the points that changed
		std::vector<point>&& not_used, // were not activated // (TODO: compute when inserting)
		std::vector<point>&& new_points,
		const debug_graph_t::vertex_t& v_n,
		Detail& detail)
	{
		bpatch_t new_changable; // patch for recent_grid

		/*
		 * refresh recent_grid
		 */

		for(const point& p : readers_of(activated))
		{
			bitgrid_t next(2, dimension(n_out.size(), 1), 0, 0);
			bool valid = ca.next_state(sim_grid, p, next);
			if(valid)
			 new_changable += bpatch_t(p, pseudo_int{next.raw_value()}, recent_grid[p]);
#ifdef DEBUG_ERRORS
			if(valid && !sim_grid.contains(p))
			 throw "ERROR: cells next to the border got active...";
#endif
		}

		new_changable.apply_fwd(recent_grid); // temporary, for this for loop iteration

		/*
		 * get active points for this next node
		 */

		// only readers of recently active points can be active now
		// (TODO: and the nodes themselves...?)
		// these are all possible active points
		std::vector<point> new_vari = my_variable_points(readers_of(cur.variable));
		detail.filter(cur, new_vari, activated);

#ifdef VERBOSE_OUTPUT
		std::cerr << "future aps: " << mk_print(new_vari) << std::endl;
#endif
		/*
		 * update dependency graph
		 */

		for(const point& p : new_changable.area())
		for(const point& changed : activated) // TODO: zip
		if(readers_of.is_neighbour_of(p, changed))
		// check if p is reader of changed = can p be activated by changed?
		{
			// reset changed's cell temporary:
			std::size_t ch_pos = 0;
			auto itr1 = bug_patch.area().begin();
			for(; itr1 != bug_patch.area().end() && *itr1 != changed; ++itr1)
			{
				++ch_pos;
			}

			std::size_t nc_pos = 0;
			auto itr2 = new_changable.area().begin();
			for(; itr2 != new_changable.area().end() && *itr2 != p; ++itr2)
			{
				++nc_pos;
			}

			// this fails in rare cases (root node)
			if(itr1 != bug_patch.area().end() && itr2 != new_changable.area().end())
			{
				sim_grid[changed] = bug_patch.old_conf()[ch_pos];
				bitgrid_t other_next(2, dimension(n_out.size(), 1), 0, 0);
				ca.next_state(sim_grid, p, other_next);
				sim_grid[changed] = bug_patch.conf()[ch_pos];

				if(other_next.raw_value() != (uint64_t)new_changable.conf()[ch_pos])
				{
					const m_dep_graph_t::edge_t e_new =
						dep_graph.try_add_edge(changed, p).first;
					dep_graph.get(e_new).set_time(stack_data::next_id());
				}
			}
		}

		new_changable.apply_bwd(recent_grid);

		/*
		 * lookup and add to stack
		 */

		{
			patch_t new_patch = (cur.patch + bug_patch);
			const auto dict_itr = dict.left.find(new_patch);

			const auto emplace_to_stack = [&]() {
				std::vector<point> activated_v;
				std::move(activated.begin(), activated.end(), std::back_inserter(activated_v));

				stack.emplace(std::move(new_patch), std::move(new_vari),
					std::move(new_points),
					std::move(activated_v), std::move(not_used),
					cur.id, new_changable, v_n);
			};

			if(dict_itr == dict.left.end())
			{
				dict.insert( dict_t::value_type(new_patch, stack_data::next_id()) );
				emplace_to_stack(); // see a few lines above...
			}
			else // node is known
			{
				const int tar_id = dict_itr->second;
#ifdef VERBOSE_OUTPUT
				std::cerr << mk_print(activated) << std::endl;
#endif
				if(tar_id == cur.id)
				 throw "error: self-cycle";
				if((cur.id > tar_id) && scc_algo.check_for_back_edge(cur.id, tar_id) )
				{
#ifdef VERBOSE_OUTPUT
					std::cerr << " -> node is known cycle to " << tar_id << std::endl;
#endif
				}
				else
				{
					stats.inform_extra_node_stack();
				}
			}
		}
	}

	//! returns all active points on @a sim_grid in area @a search
	template<class Cont>
	std::vector<point> calc_active_points(const Cont& search)
	{
		std::vector<point> new_aps;
#ifdef VERBOSE_OUTPUT
		std::cerr << "calc_active_points(): area: " << mk_print(search)
			<< std::endl;
#endif
		for(const point& p : search)
		if(ca.is_cell_active(sim_grid, p))
		 new_aps.push_back(p);
#ifdef VERBOSE_OUTPUT
		std::cerr << "ACTIVE POINTS: " << mk_print(new_aps) << std::endl;
#endif
		return new_aps;
	}


	friend io::serializer& operator<<(io::serializer& s, const brute_forcer& a);

	void m_gather_result(stack_data& cur, result_t*& last_result)
	{
		if(last_result)
		{
			if(!cur.result)
			 cur.result = res_graph.add_vertex();
			cur.result->children.push_back(last_result);

			// last_result has always the full patch from the last cur.patch
			last_result->local_patch = -cur.patch + last_result->local_patch;
			cur.result->local_patch = cur.patch;
			cur.result->vid = cur.id;

			last_result = nullptr;
		}
	}

	//! the algorithm's heart:
	//! runs with start position already set on @a sim_grid
	template<class Detail>
	result_t* run_from_start_conf(Detail& detail)
	{
		if(stack.size() != 1)
		 throw "Please call run_from_start_conf with stack size 1.";

		result_t* last_result = nullptr;

		do
		if( !stack.top().first() &&
			stack.top().next_scc >= (int)stack.top().sccs.size())
		{
			stats.dec_tree_depth();

			stack_data& cur = stack.top();
			stats.set_cur_vertex(cur.id);

			m_gather_result(cur, last_result);
#ifdef VERBOSE_OUTPUT
			std::cerr << "returning: " << cur.id << std::endl;
#endif

			{

			/*
			 * scc handling on leaving the node
			 */
			patch_t best_patch = cur.patch;
			std::size_t scc_size = 0;
			const auto& cb = [&](const int& v_scc)
			{
				++scc_size;

				// take scc with least difference to start
				auto dict_itr = dict.right.find(v_scc);
				if(dict_itr->second.size() < best_patch.size())
				 best_patch = std::move(dict_itr->second);
#ifdef DELETE_UNUSED_VERTS
				dict.right.erase(dict_itr);
#endif
			};

			scc_algo.on_finish_dfs_node(cur.id, cb);

			if((scc_size > 1) // make sccs...
				&& !cur.result // ... if we did not m_gather_result...
				&& !cur.parent_edge) // ... and no cut went out of cycle.
			{
#ifdef VERBOSE_OUTPUT
				std::cerr << " -> found scc, size: " << scc_size << "." << std::endl;
#endif
				results.final_candidates.emplace(std::set<point>(all_points), patch_t(best_patch), scc_size,
					exactness_t::greater_equal);

				cur.result = res_graph.add_vertex();
				cur.result->local_patch = best_patch; // this is a small trick, but working
				cur.result->scc_size = scc_size;
				cur.result->exactness = exactness_t::greater_equal;
				cur.result->vid = cur.id;

#ifdef DEBUG_GRAPH
				debug_graph.get(cur.v_cur).label += " final";
#endif
			}

			if(cur.src_id)
			{
				scc_algo.on_finish_new_edge(cur.src_id, cur.id);
			}

			} // scope

			/*
			 * grid cleaning
			 */

			detail.clean_up(cur);

			cur.changable.apply_bwd(recent_grid);
			cur.changable.apply_bwd(prev_grid);

			for(const point& p : cur.new_points) // TODO: zip for speedup
			 all_points.erase(p);

			last_result = cur.result; // saves result temporary
			stack.pop();
		}
		else
		{
		stack_data& cur = stack.top();
		stats.set_cur_vertex(cur.id);
		cur.patch.apply_fwd(sim_grid);

		if(cur.first())
		{
			stats.inc_tree_depth();

			// if this will be sorted out, it will not form an scc, so this is no problem
			scc_algo.discover(cur.id); // O(1)

#ifdef VERBOSE_OUTPUT
			std::cerr << "cur: " << cur.id << ", " << std::endl;
			std::cerr << "  src: " << cur.src_id << std::endl;
			std::cerr << ", grid:" << std::endl << sim_grid;
			std::cerr << "  activated: " << mk_print(cur.used) << std::endl;
#endif

			for(const point& p : cur.new_points)
			 all_points.insert(p);

			cur.changable.apply_fwd(recent_grid);

			if(detail.sort_out_2(cur.used, cur.not_used, cur.src_id))
			{
				stats.inform_isolated_point();
#ifdef DEBUG_GRAPH
#ifdef ADD_SO2
				cur.v_cur = debug_graph.add_vertex("isolated: ", true);
				debug_graph.add_edge(cur.v_src, cur.v_cur, "");
				debug_graph.get(cur.v_cur).label += to_string(cur.patch);
#endif
#endif
			}
			else
			{

				auto dict_itr = dict.left.find(cur.patch);
				if(dict_itr == dict.left.end())
				{
					throw "dict did not contain current patch, this can not happen";
				}
				else
				{
					if(cur.variable.empty()) // TODO: should be united with scc catcher
					{

						// note: this is waste for some algorithms, like greedy
						if(!calc_active_points(sim_grid.points()).size())
						{
							results.final_candidates.emplace(std::set<point>(all_points), patch_t(cur.patch));

							cur.result = res_graph.add_vertex();
							cur.result->local_patch = cur.patch;
							cur.result->scc_size = 1;
							cur.result->exactness = exactness_t::equal;
							cur.result->vid = cur.id;

#ifdef DEBUG_GRAPH
							{
								std::ostringstream ss;
								ss << "id:" << cur.id << ", " << cur.patch
									<< ", final.";

								debug_graph.add_edge(cur.v_src,
									debug_graph.add_vertex(ss.str().c_str(), true), "");
							}
#endif
						}
					}
					else
					{
						stats.inform_new_vertex(cur.patch.area());

#ifdef DEBUG_GRAPH
						{
							std::ostringstream ss;
							ss << "id:" << cur.id << ", " << cur.patch
								<< ", active: " << mk_print(cur.variable);
							std::string lbl = ss.str();

							if(cur.variable.empty())
							 lbl += ", final";

							cur.v_cur = debug_graph.add_vertex(lbl.c_str(), true);
							debug_graph.add_edge(cur.v_src, cur.v_cur, "");
						}
#endif

						detail.init_node(cur);

						// compute scc vector and first scc/bitmask
						{
							cur.sccs = detail.make_sccs(cur, cur.variable); // greedy: O(1)
							std::size_t cur_children = 0;
							for(const auto& scc : cur.sccs)
							 cur_children += (1 << scc.size());
							stats.est_cur_children(cur_children);
							cur.next_bitmask = (1 << cur.sccs[0].size()) - 1;
						}

#ifdef NEW_SCC_OUTPUT
						std::cerr << std::endl;
#endif
					}
				}
			}
			++ cur.next_scc; // start at 0 // TODO: maybe useless

		}
		else // = not the first
		{
			cur.changable.apply_bwd(prev_grid);
			m_gather_result(cur, last_result);
		}

		if(cur.sccs.size()) {
			push_next(cur, detail);
		}

		cur.changable.apply_fwd(prev_grid);
		cur.patch.apply_bwd(sim_grid);

		} while( !stack.empty() ); // = while, and also if-else (last or not)

		return last_result; // stack size was 1, so this is the return value of this node
	}

	template<class Detail>
	void push_next(stack_data& cur, Detail& detail)
	{
		const std::vector<point>& vec = cur.sccs[cur.next_scc];
		const uint64_t& bfp_mask = cur.next_bitmask;

#ifdef NEW_SCC_OUTPUT
		std::cerr << "NEW SCC" << std::endl;
#endif
		assert(vec.size() < 64);

		// we assume that there are less than 64 elements in the set
		// if they were more, we would need to execute at least 2^64 > 10^16 instructions...
		const uint64_t bfp_size = vec.size();
		{
#ifdef VERBOSE_OUTPUT
			std::cerr << "preparing: " << stack_data::next_id() << std::endl;
#endif
			// initialization, all in O(1)
			backed_up_grid bug(sim_grid);

			std::set<point> activated; // exactly the points that change
			std::vector<point> not_used; // were not activated

			std::vector<point> new_points; // points that had never changed before

			for(uint64_t bfp_pos = 0; bfp_pos < bfp_size; ++bfp_pos)
			if(bfp_mask & (1 << bfp_pos))
			{
				const point& p = vec[bfp_pos];

				activated.insert(p);
				if(all_points.find(p) == all_points.end())
				 new_points.push_back(p); // TODO: move to stats object
			}
			else
			 not_used.push_back(vec[bfp_pos]);

#ifdef VERBOSE_OUTPUT
			std::cerr << "activated points: " << mk_print(activated) << std::endl;
#endif
			bool allow = !global_abort;
			if(allow)
			{

			// checks that neighbours do not overlap
			for(const point& p : activated)
			for(const auto& np : ca::counted(n_out))
			{
				const point _p = p + (point)np;

				allow = allow && !zero_grid[_p];
				zero_grid[_p] = 1;
				bug[_p] = next_state_at(p)[point(np.id(), 0)];
			}
#ifdef VERBOSE_OUTPUT
			if(!allow)
			 std::cerr << " -> not allowed: intersecting outputs." << std::endl;
#endif
			// resets zero grid from last check
			for(const point& p : activated)
			for(const point& np : n_out)
			 zero_grid[p+np] = 0;

			bool so1 = detail.sort_out_1(cur.used, activated, bug);
			cur.parent_edge = cur.parent_edge || so1;
			allow = allow && !so1;

#ifdef DEBUG_GRAPH
#ifdef ADD_SO1
			if(so1)
			{
				debug_graph_t::vertex_t v_n = debug_graph.add_vertex("-> movable", true);
				debug_graph.add_edge(cur.v_src, v_n, "");
			}
#endif
#endif

#ifdef VERBOSE_OUTPUT
			if(!allow)
			 std::cerr << " -> not allowed: movable scc." << std::endl;
#endif

			if(allow)
			 init_new_node(cur, bug.patch(), activated, std::move(not_used), std::move(new_points), cur.v_cur, detail);
			else
			 stats.inform_movable_node();

			} // if allow

		} // scope, including backed up grid

#ifdef NEW_SCC_OUTPUT
		std::cerr << std::endl; // end of scc
#endif

		if(!--cur.next_bitmask)
		{
			++cur.next_scc;
			if(cur.next_scc < (int)cur.sccs.size()) {
				cur.next_bitmask = 1 << cur.sccs[cur.next_scc].size();
			}
		}
	}

	// this is the entry point to the algorithm
	template<class Detail>
	void run_base(Detail& detail, std::ostream& out)
	{

		total_dep_graph.init(orig_grid_unchanged.human_dim());

		for(const conf_t& c : initial_confs)
		{
			_reset_grid_to_conf(orig_grid, c, initial_area_all);
			sim_grid = orig_grid;

			recent_grid = _grid_t<char_traits, cell_traits<pseudo_int>>(sim_grid.human_dim(), sim_grid.border_width(),
				pseudo_int{0}, pseudo_int{0});
			point one = point(ca.border_width(), ca.border_width());
			for(const point& p : rect(sim_grid.human_dim().ul() - one, sim_grid.human_dim().lr() + one))
			 bit_copy(recent_grid, sim_grid, p);
			prev_grid = recent_grid;

			zero_grid = grid_t(sim_grid.human_dim(), sim_grid.border_width(), 0, 0);
			zero_grid_2 = zero_grid;

			scc_finder.init(sim_grid.human_dim());

			dep_graph.reset(sim_grid.human_dim());

			const std::vector<point> ap = calc_active_points(readers_of(initial_area_all));
			std::set<point> ap_set;
			std::copy(ap.begin(), ap.end(), std::inserter(ap_set, ap_set.begin()));

#ifdef DEBUG_GRAPH
			debug_graph_t::vertex_t v_start = debug_graph.add_vertex();
			debug_graph.add_edge_from_root(v_start);
#endif

			std::vector<point> initial_area_all_v;
			std::copy(initial_area_all.begin(), initial_area_all.end(),
				std::back_inserter(initial_area_all_v));

			// this node will never be pushed to the stack,
			// so most data in here is never used
			stack_data stack_root(
				patch_t(),
				std::vector<point>(ap), // variable points
				std::vector<point>{}, // new points ever
				std::move(initial_area_all_v), // used_points
				std::vector<point>{}, // unused_points
				0,
				bpatch_t()
#ifdef DEBUG_GRAPH
				, v_start
#else
				, brute::debug_graph_t::vertex_t {}
#endif
			);

			const detail_default def {};
			init_new_node(stack_root, patch_t(), ap_set, std::vector<point>(), std::vector<point>(), brute::debug_graph_t::vertex_t {},
				def);

			detail.init_root(stack.top());

			result_t* result = run_from_start_conf<Detail>(detail);
			if(!result)
			 throw "No result found - this is an error.";
#ifdef VERBOSE_OUTPUT
			std::cerr << "FINAL RESULT: " << *result << std::endl;
			result->dump(std::cerr);
#endif
			result->local_patch = patch_t(orig_grid, orig_grid_unchanged); // (TODO): restrict to area?
			res_graph.roots.push_back(result);

			if(!dict.empty())
			{
				for(const auto& pr : dict)
				{
					std::cerr << "dict: " << mk_print(pr.left) << " <-> " << mk_print(pr.right) << std::endl;
				}
				throw "dict should be empty";
			}
			dict.clear();

			for(auto itr = dep_graph.edges().begin(); itr != dep_graph.edges().end(); ++itr)
			 total_dep_graph.try_add_edge(itr.source(), itr.target());

		}

		std::cerr << "DUMPING RESULTS" << std::endl;
			io::serializer ser(out);
			ser << *this;
	}

	std::ostream& print(std::ostream & stream) const;

public:
	//! ctor taking a string containing the ca equation
	brute_forcer(const char* equation, cell_t border, bool dump_on_exit);

	//! ctor taking an istream to a ca table file
	brute_forcer(std::istream& is, cell_t border, bool dump_on_exit);

};

}

#endif // BRUTE_FORCE_H
