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

#ifndef GRAPH_DETAIL_H
#define GRAPH_DETAIL_H

#include <map>

//#include "contract.h"

template<class Graph, class Cont>
void copy_subgraph(Graph& res, const Graph& src, const Cont& cont)
{
	using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;
	std::map<vertex_t, vertex_t> new_id_of;
	for(const vertex_t& u : cont)
	{// TODO: store new id
		new_id_of[u] = boost::add_vertex(res);
		res[new_id_of[u]] = src[u];
	}

	auto es = boost::edges(src);
	auto itr = es.first;
	for(; itr != es.second; ++itr)
	{
		vertex_t v_src = boost::source(*itr, src),
			v_tar = boost::target(*itr, src);
		if(new_id_of.find(v_src) != new_id_of.end() && // TODO: store itr results
			new_id_of.find(v_tar) != new_id_of.end())
		{
			const auto tmp = boost::add_edge(new_id_of[v_src], new_id_of[v_tar], res);
			res[tmp.first] = src[*itr];
		}
	}
}

#endif // GRAPH_DETAIL_H
