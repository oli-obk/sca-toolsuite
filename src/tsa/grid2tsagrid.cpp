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

#include <cstdlib>
#include <cstdio>
#include <vector>

#include <boost/graph/graph_traits.hpp>

#include <boost/graph/transitive_reduction.hpp>
//#include <boost/graph/transitive_closure.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp>

#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include <boost/graph/strong_components.hpp>

#include "general.h"
#include "io.h"
#include "stack_algorithm.h"

void create_A(
	graph_t* A,
	const std::vector<int>* grid,
	const dimension* dim)
{
	unsigned int area = dim->area_without_border();
	std::vector<int> workGrid;
	ArrayQueueNoFile container(area);

	for(unsigned int i=0; i<area;i++)
	 boost::add_vertex(*A);

	for(unsigned int cur_node=0; cur_node<area; cur_node++)
	{
		workGrid = *grid;
		int idx = human2internal(cur_node, dim->width);
		// shouldn't it be 4? no! see docs about avalanche_1d_hint_noflush
		workGrid[idx]=3;
//		write_grid(stdout, &workGrid, dim);
		avalanche_1d_hint_noflush(&workGrid, dim, idx, &container, NULL);

		for(unsigned int j=0; j<container.size(); j++)
		{
			int from_node = cur_node;
			int to_node = internal2human(container.data()[j], dim->width);
			if(to_node != from_node) // avoid loops
			 boost::add_edge(from_node, to_node, *A);
		}
		container.flush();
	}

}


void create_S(graph_t* S, graph_t* A)
{
	std::vector<int>
		component(num_vertices(*A)),
		discover_time(num_vertices(*A));

//	const int num = strong_components(*A, &component[0]);
TODO: const int num = boost::strong_components(*A,
		      			boost::make_iterator_property_map(component.begin(), boost::get(boost::vertex_index, *A), component[0]));

	// now, create new graph
	for(unsigned int i=0; i<num;i++)
	 boost::add_vertex(*S);

	// get the property map for vertex indices
	typedef property_map<graph_t, vertex_index_t>::type IndexMap;
	IndexMap index = get(vertex_index, *A);

	graph_traits<graph_t>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(*A); ei != ei_end; ++ei)
	{
		const int source_comp = component[index[source(*ei, *A)]];
		const int target_comp = component[index[target(*ei, *A)]];

		if(source_comp != target_comp)
		 add_edge(source_comp, target_comp,*S);
	}

	assert(boyer_myrvold_planarity_test(*S));
}

void transitive_closure(graph_t* out_graph, const graph_t* in_graph)
{
#if 0
	boost::transitive_closure(*in_graph, *out_graph);
#endif
}

typedef graph_traits<graph_t>::edge_descriptor edge_descr;
struct edge_index_cmp : std::binary_function<edge_descr, edge_descr, bool>
{
	const graph_t& graph;
	edge_index_cmp(const graph_t& _graph) : graph(_graph) {}
	bool operator()(edge_descr& e1, edge_descr& e2)
	{
		typedef property_map<graph_t, vertex_index_t>::type vidx_map;
		vidx_map imap1 = get(vertex_index, graph);
		return imap1[target(e1, graph)] < imap1[target(e2, graph)];
	}
};

//! TODO: we should not use graph_t here, but templates instead...
void transitive_reduction_of_dag(graph_t* out_graph, const graph_t* in_graph)
{
#if 0
	typedef graph_traits<graph_t>::vertex_iterator vitr;
	/*std::pair<vertex_iter, vertex_iter> vp;
	int* ptr = result.data();
	for (vp = vertices(S); vp.first != vp.second; ++vp.first)*/

	std::pair<vitr, vitr> range;
	for (vitr itr = range.first; itr != range.second; ++itr)
	for (vitr itr = range.first; itr != range.second; ++itr)
	for (vitr itr = range.first; itr != range.second; ++itr)
	if edges xy and yz exist
	delete edge xy
#endif
#if 0
	graph_t trans_cl, in_copy = *in_graph;
	transitive_closure(trans_cl, in_copy);

	graph_t trans_cl_copy;

	typedef property_map<graph_t, vertex_index_t>::type vidx_map;

/*	typedef indirect_cmp<vidx_map, std::greater<vidx_map::value_type> > idx_greater;
	vidx_map imap1 = get(vertex_index, trans_cl);
	idx_greater compare(imap1);
	typedef graph_traits<graph_t>::edge_descriptor edge_descr;

	std::priority_queue<edge_descr, std::vector<edge_descr>, idx_greater> idx_q;
*/
	edge_index_cmp cmp(trans_cl_copy);
	std::priority_queue<edge_descr, std::vector<edge_descr>, edgeFirstIdxCompare> idx_q(trans_cl_copy);

	/*push all edge into q*/
	typename graph_traits<graph_t>::edge_iterator ei, eiend;
	for (boost::tie(ei, eiend) = edges(trans_cl_copy); ei != eiend; ++ei)
	 idx_q.push(*ei);
#endif

}

template <typename in_type, typename out_type>
void const_transitive_reduction(in_type* out_graph, const out_type* in_graph)
{
/*	//typedef typename property_map<Graph, vertex_index_t>::const_type VertexIndexMap;
	//VertexIndexMap index_map = get(vertex_index, in_graph);

	typedef typename graph_traits<in_type>::vertex_descriptor tc_vertex;
	std::vector<tc_vertex> to_tc_vec(num_vertices(*in_graph));

//	iterator_property_map <tc_vertex *, VertexIndexMap, tc_vertex, tc_vertex&>
//	g_to_tc_map(&to_tc_vec[0], index_map);

	out_type tmp_out_graph;
	boost::transitive_reduction(*in_graph, tmp_out_graph, g_to_tc_map);
*/}

void create_T(graph_t* T, graph_t* S)
{

/*foreach x in graph.vertices
   foreach y in graph.vertices
      foreach z in graph.vertices
      if (x,z) != (x,y) && (x,z) != (y,z)
	 delete edge xz if edges xy and yz exist*/

/*	//IndexMap index2 = get(vertex_index, tr_graph);
	typedef graph_traits<graph_t>::vertex_iterator vertex_iter;
	std::pair<vertex_iter, vertex_iter> vp;
	int* ptr = result.data();
	for (vp = vertices(S); vp.first != vp.second; ++vp.first) {
	}
	*T = *S;
	*/
	transitive_reduction_of_dag(T, S);
}

int main(int argc, char** argv)
{
	(void)argv;
	FILE* read_fp = stdin;
	if(argc>1) {
		fputs("There are no arguments for this tool.\n", stderr);
		exit(1);
	}

	std::vector<int> grid;
	dimension dim;
	read_grid(read_fp, &grid, &dim);

	graph_t T, S, A;
	create_A(&A, &grid, &dim);
	create_S(&S, &A);
	create_T(&T, &S);

	dump_graph_as_tgf(stdout, &A);

	return 0;
}
