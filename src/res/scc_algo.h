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

#ifndef SCC_ALGO_H
#define SCC_ALGO_H

#include <vector>
#include <map>
#include <iostream> // TODO!

//#define SCC_ALGO_DEBUG

//! implementation of tarjan to find sccs *while* running a dfs
template<class vertex_t>
class scc_algo_t
{
	struct node_data_t
	{
		int index;
		int lowlink;
		node_data_t(int idx) noexcept : index(idx), lowlink(idx) {}
		node_data_t() noexcept {}
	};

	std::map<vertex_t, node_data_t> node_data; //!< keeps the data
	std::vector<vertex_t> stack; //!< keeps the order

	int last_index = -1;

	//! complexity: log(n)
	node_data_t& find(const vertex_t& v) {
		return node_data.find(v)->second; }
	//! complexity: log(n)
	const node_data_t& find(const vertex_t& v) const {
		return node_data.find(v)->second; }

public:
	//! When you discover a vertex, before recursing on its children
	//! complexity: log(n), O(1) if ordered
	void discover(const vertex_t& v_cur)
	{
#ifdef SCC_ALGO_DEBUG
		std::cerr << "scc algo: discover: " << v_cur << std::endl;
#endif

		int new_index = ++last_index;
		node_data.emplace(v_cur, node_data_t(new_index)); // TODO: emplace_hint
		/*if(stack.size() && v_cur <= stack.back())
		{
			std::cout << stack.back() << std::endl;
			std::cout << v_cur << std::endl;
			throw "discover";
		}*/
		stack.push_back(v_cur);
	}

	//! When an edge has been finished from the dfs, not via a back edge
	//! @note Must be called before @a on_finish_dfs_node
	//! complexity: log(n)
	void on_finish_new_edge(const vertex_t& v_src, const vertex_t& v_tar)
	{
#ifdef SCC_ALGO_DEBUG
		std::cerr << "scc algo: on_finish_new_edge: " << v_src << ", " << v_tar << std::endl;
#endif
		auto& src = find(v_src);
		const auto& tar = find(v_tar);
		src.lowlink = std::min(src.lowlink, tar.lowlink);
	}

	//! Checks whether connecting to v_tar means drawing a cycle
	//! complexity: log(n)
	bool is_cycle_node(const vertex_t& v_tar) const
	{
		return node_data.find(v_tar) != node_data.end();
	}

	//! The back edge has not been recursed on. Now call this.
	//! @deprecated deprecated!
	//! complexity: log(n)
	void on_back_edge(const vertex_t& v_src, const vertex_t& v_tar)
	{
		if(is_cycle_node(v_tar))
		{ // <=> v_tar found in stack
			auto& src = find(v_src);
			const auto& tar = find(v_tar);
			src.lowlink = std::min(src.lowlink, tar.index);
		}
	}

	//! The back edge has not been recursed on. Now call this.
	//! complexity: log(n)
	bool check_for_back_edge(const vertex_t& v_src, const vertex_t& v_tar)
	{
#ifdef SCC_ALGO_DEBUG
		std::cerr << "scc algo: check_for_back_edge: " << v_src << ", " << v_tar << std::endl;
#endif
		if(is_cycle_node(v_tar)) // TODO: duplicate map lookup!
		{ // <=> v_tar found in stack
			auto& src = find(v_src);
			const auto& tar = find(v_tar);
			src.lowlink = std::min(src.lowlink, tar.index);
#ifdef SCC_ALGO_DEBUG
			std::cerr << "found back edge: " << v_src << " -> " << v_tar << std::endl;
#endif
			return true;
		}
		else return false;
	}

	//! After a node has been left.
	//! @note Must be called after @a on_finish_new_edge
	//! complexity: worst case: n, amortized: log(n)
	template<class Functor>
	void on_finish_dfs_node(const vertex_t& v_cur, const Functor& ftor)
	{
#ifdef SCC_ALGO_DEBUG
		std::cerr << "scc algo: on_finish_dfs_node: " << v_cur << std::endl;
#endif
		const auto& cur = find(v_cur); // O(log(n))
		if(cur.lowlink == cur.index)
		{
			vertex_t back;
			do // O(1) amortized
			{
				ftor(back = stack.back());
				stack.pop_back();
				node_data.erase(back);
			} while(back != v_cur);
		}
	}
};

#endif // SCC_ALGO_H
