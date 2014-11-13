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

#include "stats.h"

#define STATS_DUMP_USED_GRID

void stats_t::dump() const
{
	std::cerr << "STATS:" << std::endl;
	std::cerr << "current vertex id:" << cur_vertex << std::endl;
	std::cerr << "number of vertices: " << n_verts << std::endl;
	std::cerr << "movable nodes (=cuts): " << movable_nodes << std::endl;
	std::cerr << "isolated points (=cuts): " << isolated << std::endl;
	std::cerr << "extra nodes from stack (=cuts): " << extra_stack << std::endl;
	std::cerr << "extra nodes global: ";
	if(has_extra_nodes)
		std::cerr <<  extra_nodes << std::endl;
	else
		std::cerr << "(not counted)" << std::endl;
	bounding_box bb; // TODO: ctor from point container
	for(const point& p : super_area)
		bb.add_point(p);
#ifdef STATS_DUMP_USED_GRID
	grid_t g(dimension(bb.lr().x, bb.lr().y), 0);
	g[super_area] = 1;
	std::cerr << "all used cells:" << std::endl << g << std::endl;
#endif
	{
#if 0
		std::size_t nodes_done;
		double nodes_to_do = 1;
		for(const depth_wise_t& dw : at_depth)
		{
			/*std::size_t
			nodes_to_do *= */
//			std::cerr << dw.nodes << " <-> " << dw.waiting_nodes << std::endl;
		}
#endif
	}


	//std::cerr << "progress: ";
}
