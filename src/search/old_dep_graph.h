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

#ifndef OLD_DEP_GRAPH_H
#define OLD_DEP_GRAPH_H

#include <string>
#include "geometry.h"
#include "graph/graph.h"

template<class Point>
struct dep_v_t
{
	Point p;
	std::string label;
	dep_v_t(const Point& p) :
		p(p),
		label(std::to_string(p.x) + " " + std::to_string(p.y))
		{}
	dep_v_t() {}
};
struct dep_e_t
{
	int time;
	std::string label;
	void set_time(int _time) {
		time = _time;
		label = std::to_string(time);
	}
};

template<class Point>
using dep_graph_t = graph_base_t<dep_v_t<Point>, dep_e_t>;

#endif // OLD_DEP_GRAPH_H
