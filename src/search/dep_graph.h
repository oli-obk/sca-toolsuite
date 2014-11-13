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

#ifndef DEP_GRAPH_H
#define DEP_GRAPH_H

#include <map>
#include "common_macros.h"

class new_dep_graph_t : public types
{
public:
	struct edge
	{
		bool exists = false;
		int time = 0; // any default value would fit
		void set_time(int _time) { time = _time; }
		bool operator==(const edge& other) const {
			return exists == other.exists
				&& time == other.time;
			}
		edge(){}
	};
private:
	using dep_grid_t = _grid_t<char_traits, cell_traits<std::map<point, edge>>>;
	dep_grid_t links;
	const n_t& n;
	std::size_t _num_edges = 0;
public:

	void reset(const dimension& human_dim) {
		_num_edges = 0;
		init(human_dim);
	}

	using edge_t = edge*;
	using const_edge_t = const edge*;

	new_dep_graph_t(const char*, const n_t& n) :
		n(n)
	{
	}

	void init(const dimension& human_dim)
	{
		std::map<point, edge> dummy;
		for(const auto& p : n)
		 dummy[p];
		links = dep_grid_t(human_dim, 0, dummy);
	}

	std::pair<edge_t, bool> try_add_edge(const point&p, const point& p_dest)
	{
		const point diff = p_dest - p;
		edge& e = links[p][diff];

		std::pair<edge_t, bool> rval = {&e, !e.exists};
		_num_edges += !e.exists;
		e.exists = true;
		return rval;
	}

	const edge& get(const const_edge_t& e) const { return *e; }
	edge& get(const edge_t& e) { return *e; }

	std::size_t num_edges() const { return _num_edges; }

	class itr_base
	{
	protected:
		const new_dep_graph_t& gr;
		point_itr pitr;
		n_t::const_iterator nitr;
	public:
		itr_base(const new_dep_graph_t& gr, const point_itr& pitr, n_t::const_iterator&& nitr) :
			gr(gr),
			pitr(pitr),
			nitr(nitr)
		{
		}
		const_edge_t operator*() const {
			return &(gr.links[*pitr].find(*nitr)->second);
		}

		point source() const { return *pitr; }
		point target() const { return *pitr + *nitr; }
	};

	class const_out_edge_itr : public itr_base
	{
	public:
		const_out_edge_itr(const new_dep_graph_t& _gr, const point_itr& _pitr, n_t::const_iterator&& _nitr) :
			itr_base(_gr, _pitr, std::move(_nitr))
		{
			while(nitr != gr.n.cend() && !gr.links[*pitr].find(*nitr)->second.exists)
			 ++nitr;
		}

		const_out_edge_itr& operator++()
		{
			do
			{
				++nitr;
			} while(nitr != gr.n.cend() && !gr.links[*pitr].find(*nitr)->second.exists);
			return *this;
		}
		const_out_edge_itr& operator=(const const_out_edge_itr& other) = default;
		bool operator==(const const_out_edge_itr& other) const { return &gr == &other.gr && pitr == other.pitr && nitr == other.nitr; }
		bool operator!=(const const_out_edge_itr& other) const { return !operator==(other); }
	};


	class const_edge_itr : public itr_base
	{
	public:
		//using itr_base::itr_base;
		const_edge_itr(const new_dep_graph_t& _gr, const point_itr& _pitr, n_t::const_iterator&& _nitr) :
			itr_base(_gr, _pitr, std::move(_nitr))
		{
			while(pitr != gr.links.points().end()
				&& !gr.links[*pitr].find(*nitr)->second.exists)
			{
				if(++nitr == gr.n.end())
				{
					++pitr;
					nitr = gr.n.cbegin();
				}
			}
		}
		const_edge_itr& operator++()
		{
			do
			{
				if(++nitr == gr.n.end())
				{
					++pitr;
					nitr = gr.n.cbegin();
				}
			} while(pitr != gr.links.points().end() && !gr.links[*pitr].find(*nitr)->second.exists);
			return *this;
		}
		const_edge_itr& operator=(const const_edge_itr& other) = default;
		bool operator==(const const_edge_itr& other) const { return &gr == &other.gr && pitr == other.pitr && nitr == other.nitr; }
		bool operator!=(const const_edge_itr& other) const { return !operator==(other); }
	};

	class const_edge_cont
	{
		const new_dep_graph_t& gr;
	public:
		const_edge_cont(const new_dep_graph_t& gr) :
			gr(gr) {}
		const_edge_itr begin() const { return { gr, gr.links.points().cbegin(), gr.n.cbegin() }; }
		const_edge_itr end() const { return { gr, gr.links.points().cend(), gr.n.cbegin() }; }
	};

	class const_out_edge_cont
	{
		const new_dep_graph_t& gr;
		const point& p;
	public:
		const_out_edge_cont(const new_dep_graph_t& gr, const point& p) :
			gr(gr), p(p) {}
		const_out_edge_itr begin() const { return { gr, gr.links.points().cbegin() + p, gr.n.cbegin() }; }
		const_out_edge_itr end() const { return { gr, gr.links.points().cbegin() + p, gr.n.cend() }; }
	};

	bool operator==(const new_dep_graph_t& other) const { return links == other.links /*&& n == other.n*/; }
	const const_edge_cont edges() const { return {*this}; }
	const const_out_edge_cont out_edges(const point& p) const { return {*this, p}; }
};

#endif // DEP_GRAPH_H
