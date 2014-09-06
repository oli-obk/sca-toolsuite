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

#ifdef SCA_GRAPHS

#include "graph_io.h"

void create_boost_graph(FILE* read_fp, graph_t* boost_graph)
{

	unsigned int num_nodes = 0;
	unsigned int width, height;
	fread(&width, 4, 1, read_fp);
	fread(&height, 4, 1, read_fp);
	num_nodes = (width-2) * (height-2);
	for(unsigned int i=0; i<num_nodes;i++)
	 boost::add_vertex(*boost_graph);
//	 fprintf(write_fp, "%d v_%d\n",i,i);


//	fprintf(write_fp, "#\n");

	unsigned int current_node = 0;
	int current_int;
	while(current_node < num_nodes)
	{
		{
			const int num_read = fread(&current_int, 4, 1, read_fp);
			assert(num_read > 0);
		}
		if(current_int == -1)
		{
			current_node++;
		}
		else
		{
			const unsigned int to_node = internal2human(current_int,width);
			if(to_node != current_node) // avoid loops
			 boost::add_edge(current_node, to_node, *boost_graph);
			//printf("read edge %d %d\n",current_node, to_node);
			// fprintf(write_fp, "%d %d\n", current_node, to_node);
		}
	}
}

void dump_graph_as_tgf(FILE* write_fp, const graph_t* graph)
{
	// dump nodes
	for(unsigned int i=1; i<=num_vertices(*graph);i++)
	 fprintf(write_fp, "%d v_%d\n",i,i);

	// get the property map for vertex indices
	typedef property_map<graph_t, vertex_index_t>::type IndexMap;
	IndexMap index = get(vertex_index, *graph);

	// dump seperator
	fprintf(write_fp, "#\n");

	// dump edges
	graph_traits<graph_t>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(*graph); ei != ei_end; ++ei)
	{
		const int cur_source = index[source(*ei, *graph)];
		const int cur_target = index[target(*ei, *graph)];
		fprintf(write_fp, "%d %d\n", cur_source+1, cur_target+1);
	}
}

#endif
