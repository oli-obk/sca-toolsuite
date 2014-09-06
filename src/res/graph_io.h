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

#ifndef GRAPH_IO_H
#define GRAPH_IO_H

#ifdef SCA_GRAPHS

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

typedef boost::adjacency_list<
		vecS,vecS,directedS, property<vertex_index_t, int>
	> graph_t;
//! to be documented
void create_boost_graph(FILE* fp, graph_t* boost_graph);

//! dumps a boost graph into a trivial graphics file (TGF, see Wikipedia)
void dump_graph_as_tgf(FILE* write_fp, const graph_t* boost_graph);

/*//! dumps a boost graph into a PDF file, using graphviz
template<class GraphType>
void dump_graph_as_pdf(std::ostream& stream, const GraphType& boost_graph);

//! dumps a boost graph into a PDF file, using graphviz
//! if @a fname is nullptr, the filename is chosen at random
template<class GraphType>
void dump_graph_as_pdf(const GraphType &boost_graph, const char* fname = nullptr);
*/
template<class GraphType>
void dump_graph_as_dot(const GraphType &graph, std::ostream &stream, const boost::dynamic_properties& dp)
{
//	boost::dynamic_properties dp;
//	dp.property("label", boost::get(&Vertex::name, g));
//	dp.property("node_id", get(boost::vertex_index, graph)); //TODO: boost::get?

	// TODO: separate write func?
	write_graphviz_dp(stream, graph, dp);
}

template<class GraphType>
void dump_graph_as_dot(const GraphType &graph, const boost::dynamic_properties& dp, const char* fname = nullptr
	)
{
	const char* filename;
	if(fname)
		filename = fname;
	else
	{
		char temp[] = "graphXXXXXX";
		mkstemp(temp);
		filename = temp;
	}

	std::ofstream stream(filename);
	dump_graph_as_dot(graph, stream, dp);
}

#endif SCA_GRAPHS

#endif // GRAPH_IO_H
