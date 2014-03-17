/*************************************************************************/
/* sca toolsuite - a toolsuite to simulate cellular automata         .   */
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

#include "general.h"
#include "io.h"

//#include <boost/graph/detail/permutation.hpp> // NEW
#include <boost/graph/graph_traits.hpp>

#include <boost/graph/transitive_reduction.hpp>
//#include <boost/graph/transitive_closure.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp>

#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include <boost/graph/strong_components.hpp>







/*int permute_with(std::vector<int> result, std::vector<int> permutation)
{
	if(result.size() != permutation.size()) {
		fputs(stderr, "Permutation of array with different size.\n");
		exit(1);
	}

	int cur_outswapped_no = 0;
	int cur_outswapped;
	do {
		cur_outswapped = result[cur_outswapped_no];
		int next = permutation[cur_outswapped_no];
		result[cur_outswapped_no] = result[next];
		cur_outswapped_no = next;
	} while(cur_outswapped_no);
}*/

void permute_with(std::vector<int>& input, std::vector<int>& result, std::vector<int> permutation)
{
	if(input.size() != permutation.size()) {
		fputs("Permutation of array with different size.\n", stderr);
		exit(1);
	}
	result.resize(input.size());

	std::vector<int>::const_iterator itr = input.begin();
	std::vector<int>::const_iterator perm = permutation.begin();
	for(; itr != input.end(); itr++, perm++)
	{
		result[(*perm)-1] = *itr;
	}
}



template <typename Graph, typename GraphTC>
void transitive_reduction(const Graph & g, GraphTC & tc)
{
	using namespace boost;
	if (num_vertices(g) == 0)
	 return;

	typedef typename property_map<Graph, vertex_index_t>::const_type VertexIndexMap;
	VertexIndexMap index_map = get(vertex_index, g);

	typedef typename graph_traits<GraphTC>::vertex_descriptor tc_vertex;
	std::vector<tc_vertex> to_tc_vec(num_vertices(g));

	iterator_property_map < tc_vertex *, VertexIndexMap, tc_vertex, tc_vertex&>
	g_to_tc_map(&to_tc_vec[0], index_map);

	transitive_reduction(g, tc, g_to_tc_map, index_map);

	//	permute(vertices(tc), vertices(tc)+vertices(tc).size(), g_to_tc_map);

	/*while (permuter != last) {
		std::swap(result[n], result[*permuter]);
		++n;
		++permuter;
	}*/


	typedef property_map<graph_t, vertex_index_t>::type IndexMap;
	IndexMap index2 = get(vertex_index, g);

	typedef graph_traits<graph_t>::vertex_iterator vertex_iter;
	std::pair<vertex_iter, vertex_iter> vp;
	/*int* ptr = result.data();
	for (vp = vertices(tr_graph); vp.first != vp.second; ++vp.first) {

		fprintf(write_fp, "%d ", index2[*vp.first]+1); // tgf wants in [1,...]

		while(*ptr != -1)
		 fprintf(write_fp, "%d ", *(ptr++));
		fprintf(write_fp,"\n");
		ptr++;
	}*/

	std::vector<int> tmp_vec;
//	permute_with(vertices(g), tmp_vec, g_to_tc_map);
}



/*void test(void)
{
	using namespace boost;
	typedef property < vertex_name_t, char >Name;
	typedef property < vertex_index_t, std::size_t, Name > Index;
	typedef adjacency_list < listS, listS, directedS, Index > graph_t_2;
	typedef graph_traits < graph_t_2 >::vertex_descriptor vertex_t;
	graph_t_2 G;
	std::vector < vertex_t > verts(4);
	for (int i = 0; i < 4; ++i)
	  verts[i] = add_vertex(Index(i, Name('a' + i)), G);
	add_edge(verts[1], verts[2], G);
	add_edge(verts[1], verts[3], G);
	add_edge(verts[2], verts[1], G);
	add_edge(verts[3], verts[2], G);
	add_edge(verts[3], verts[0], G);

	std::cout << "Graph G:" << std::endl;
	print_graph(G, get(vertex_name, G));

	adjacency_list <> TC;
	transitive_reduction(G, TC);

}*/

/**
	input: component array from scc algo
	resulting format: n1 n2 n3 -1 n4 n5 -1 .....
*/
void make_node_string(std::vector<int>& component, int human_grid_size, int component_size, std::vector<int>& result) {
	// get number of decimal signs for this string
//	int dec_num = 0;
//	for(int cur_size = human_grid_size; cur_size > 1; dec_num++, cur_size/=10) ;
//	node_string.reserve(human_grid_size * (dec_num+1));


//	printf("comp size: %d, grid size: %d\n",component_size,human_grid_size);
	// part 1: get the numbers in each component:
	std::vector<int> num_per_component(component_size, 0);
	for(int i = 0; i < human_grid_size; i++) {
		num_per_component[component[(i)]]++;
	}
	for(int i = 0; i < component_size; i++) {
		printf("npc: %d \n",num_per_component[i]);
	}

	// part 2: fill
	result.resize(human_grid_size+component_size);
	std::vector<int*> write_pointers(component_size);
	write_pointers[0] = result.data();
	for(int i = 1; i < component_size; i++) {
		write_pointers[i] = write_pointers[i-1]+num_per_component[i-1]+((i==1)?0:1);
	}
	for(int i = 1; i < component_size; i++) {
		(*write_pointers[i]++)=-1;
	}

	for(int i = 0; i<human_grid_size; i++) {
		(*write_pointers[component[i]]++)=(i);
	}
	result[result.size()-1] = -1;
}

int main(int argc, char** argv)
{
	FILE* read_fp = stdin;
	FILE* write_fp = stdout;
	if(argc>1) {
		fputs("There are no arguments for this tool.\n", stderr);
		exit(1);
	}

	/*
		STEP 1 : build avalanche graph
	*/
	graph_t boost_graph;
	create_boost_graph(read_fp, &boost_graph);

	/*
		STEP 2 : build SCC graph
	*/
	std::vector<int> component(num_vertices(boost_graph)), discover_time(num_vertices(boost_graph));
	const int num = strong_components(boost_graph, &component[0]);

	// now, create new graph
	graph_t scc_graph(num);




	// get the property map for vertex indices
	typedef property_map<graph_t, vertex_index_t>::type IndexMap;
	IndexMap index = get(vertex_index, boost_graph);

	graph_traits<graph_t>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(boost_graph); ei != ei_end; ++ei) {
		const int source_comp = component[index[source(*ei, boost_graph)]];
		const int target_comp = component[index[target(*ei, boost_graph)]];
		if(source_comp != target_comp)
		add_edge(
		source_comp, target_comp,
		scc_graph);
//		printf("v %d in comp %d\n",index[source(*ei, boost_graph)], component[index[source(*ei, boost_graph)]]);
	}

	assert(boyer_myrvold_planarity_test(scc_graph));

	/*
		STEP 3 : transitive reduction
	*/

	//print_graph(scc_graph, get(vertex_name, scc_graph));
	//graph_t& tr_graph = scc_graph;
	graph_t tr_graph;
	transitive_reduction(scc_graph, tr_graph);

	/*
		write output
	*/
	std::vector<int> result;
	make_node_string(component, num_vertices(boost_graph), num_vertices(tr_graph), result);
	/*for(int i =0; i < result.size(); i++) {
		printf("comp: %d\n", result[i]);
	}*/

	IndexMap index2 = get(vertex_index, tr_graph);
	typedef graph_traits<graph_t>::vertex_iterator vertex_iter;
	std::pair<vertex_iter, vertex_iter> vp;
	int* ptr = result.data();
	for (vp = vertices(tr_graph); vp.first != vp.second; ++vp.first) {

		fprintf(write_fp, "%d ", index2[*vp.first]+1); // tgf wants in [1,...]

		while(*ptr != -1)
		 fprintf(write_fp, "%d ", *(ptr++));
		fprintf(write_fp,"\n");
		ptr++;
	}
	fputs("#\n", write_fp);

	//graph_traits<graph_t>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(tr_graph); ei != ei_end; ++ei) {
		printf("%d %d\n",index2[source(*ei, tr_graph)]+1,
		index2[target(*ei, boost_graph)]+1);
	}

	return 0;
}


