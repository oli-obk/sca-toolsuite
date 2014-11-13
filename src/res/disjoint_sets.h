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

#ifndef DISJOINT_SETS_H
#define DISJOINT_SETS_H

#include <vector>
#include <map>
#include <stack>

#include <boost/pending/disjoint_sets.hpp>

#include "ca_basics.h"
#include "geometry.h"

// TODO: conf_t bool?
/**
 * Complexity: O(marked * n2 * alpha(marked * n2, _area))
 * @param marked boolean configuration, with 1 if this should be marked. Size must be @a n2( @a area)
 * @param _area Area of points. Needs to have exactly the same size as @a marked.
 * @param n2 metric ...
 * @return note: the vector is ordered
 */
template<class Conf, class Cont, class NClass>
std::map<std::size_t, std::vector<typename Cont::value_type>> get_scc_areas(
	const Conf& marked,
	const Cont& _area, const NClass& n2)
{
	using point = typename Cont::value_type;

	std::vector<int> rank(_area.size());
	std::vector<int> parent(_area.size());
	boost::disjoint_sets<int*, int*> d_sets(&rank[0], &parent[0]);

	auto itr = _area.begin();

	for(std::size_t i = 0; i < _area.size(); ++i, ++itr)
	 d_sets.make_set((int)i);

	itr = _area.begin();

	// O(marked * n2 * alpha(marked * n2, _area))
	for(std::size_t i = 0; i < marked.size(); ++i, ++itr)
	{
		const point& p = *itr;

		if(marked[i])
		for(const auto& np : n2)
		{
			const point pnp = p + point(np.x, np.y);

			auto itr2 = _area.begin();
			std::size_t j = 0;
			// TODO: slow algorithm here
			for( ; j < _area.size(); ++j, ++itr2)
			{
				if(*itr2 == pnp)
				 break;
			}

			if(itr2 != _area.end() && marked[j])
			 d_sets.union_set((int)i, (int)j);
		}
	}

	itr = _area.begin();

	std::map<std::size_t, std::vector<point>> results;
	for(std::size_t i = 0; i < parent.size(); ++i, ++itr)
	{
		if(marked[i])
		{
			results[parent[i]].push_back(*itr);
		}
	}

	return results;
}

//! Complexity: O(marked * n2 * alpha(marked * n2, _area))
template<class Conf, class Cont, class NClass>
std::vector<std::vector<typename Cont::value_type>> get_scc_areas_v(
	const Conf& marked,
	const Cont& _area, const NClass& n2)
{
	auto map = get_scc_areas(marked, _area, n2);
	std::vector<std::vector<typename Cont::value_type>> res;
	for(const auto& pr : map)
	 res.push_back(std::move(pr.second));
	return res;
}

//! Complexity: O(_area * n2 * alpha(_area * n2, _area))
template<class Cont, class NClass>
std::vector<std::vector<typename Cont::value_type>> get_scc_areas_v_all(
	const Cont& _area, const NClass& n2)
{
	const sca::ca::_conf_t<cell_traits<unsigned char>>
		tmp(1, _area.size());
	return get_scc_areas_v(tmp, _area, n2);
}

template<class Grid>
class grid_scc_finder_t
{
	mutable Grid _visited_grid;
	using point = typename Grid::point;

	// TODO: non recursive
	//! grid dfs for the one component starting at p
	//! complexity: O(n + T_abort)
	template<class NClass, class Functor>
	void grid_dfs(
		const typename Grid::point& p,
		const NClass& n,
		const Functor& cb_abort,
		std::size_t id) const
	{
		_visited_grid[p] = id;
		for(const point& np : n)
		{
			const point next = p + np;
			if(_visited_grid.contains(next) // be careful
				&& (!_visited_grid[next]) && !cb_abort(p, next))
			 grid_dfs(next, n, cb_abort, id);
		}
	}

	//! grid dfs for area. result is implicit in _visited_grid
	//! complexity: O(area * (n + abort))
	template<class NClass, class Cont, class Functor>
	std::size_t grid_dfs(
		const NClass& n,
		const Cont& area,
		const Functor& cb_abort) const
	{
		_visited_grid.reset(0);

		// each point of area is only visited once
		// -> complexity O(area * (n + abort))
		std::size_t last_id = 0;
		for(const point& p : area)
		if((!_visited_grid[p]) /*&& !cb_abort(p)*/) // abort is not called for points in area
		 grid_dfs(p, n, cb_abort, ++last_id);

		return last_id;
	}

public:
	static void no_cb_size(const std::size_t& ) {}

	//! grid dfs for area. result is implicit in _visited_grid
	//! complexity: O(area * (n + abort))
	template<class NClass, class Cont, class Functor, class Functor2,
		class Functor3 = decltype(no_cb_size)>
	std::size_t operator()(
		const NClass& n,
		const Cont& ini_area,
		const Functor& cb_abort,
		const Functor2& cb_action,
		const Functor3& cb_size = no_cb_size) const
	{
		std::size_t num = grid_dfs(n, ini_area, cb_abort);
	//	std::cerr << "VIS:" << visited_grid << std::endl;
		cb_size(num);
		for(const point& p : ini_area)
		{
			const auto& comp_id = _visited_grid[p];
			assert(comp_id != 0);
			cb_action(p, comp_id - 1);
		}
		return num;
	}

	//! grid dfs, returning result in a 2D vector
	//! complexity: O(ini_area * (n + abort))
	template<class Cont, class NClass, class Functor>
	std::vector<std::vector<typename Grid::point>>
		operator()(const NClass& n, const Cont& ini_area, const Functor& cb_abort) const
	{
		// 1 = in scc
		// 0 = out of scc or discard

		std::vector<std::vector<point>> res;
		const auto cb_push = [&](const point& p, const std::size_t& comp_id)
		{
			res[comp_id].push_back(p);
		};
		const auto cb_size = [&](const std::size_t& sz)
		{
			res.resize(sz);
		};

		operator()(n, ini_area, cb_abort, cb_push, cb_size);

		return res;
	}

public:
	grid_scc_finder_t(const _dimension<typename Grid::traits_t>& human_dim)
		: _visited_grid(human_dim, 0, 0)
	{}

	grid_scc_finder_t() = default;

	void init(const _dimension<typename Grid::traits_t>& human_dim)
	{
		// TODO: version with borders?
		_visited_grid = Grid(human_dim, 0, 0);
	}

	const Grid& visited_grid() const { return _visited_grid; }

/*	//! complexity: O(area * (n + abort))
	template<class Cont, class NClass, class Functor>
	std::vector<std::vector<typename Grid::point>> operator()(
		const NClass& n, const Cont& area, const Functor& cb_abort
		) const {
		return get_scc_areas_grid(n, area, cb_abort);
	}*/
};

#endif // DISJOINT_SETS_H
