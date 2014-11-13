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

#ifndef GREEDY_H
#define GREEDY_H

#include "brute_force.h"

namespace greedy
{

class algo : public brute::brute_forcer
{
	struct greedy_detail_t : detail_default {

		bool sort_out_1(const std::vector<point>& cur_used, const std::set<point>& activated, const backed_up_grid& bug) const
		{
			return al.one_scc_of_movables(cur_used, activated, bug);
		}

		bool sort_out_2(const std::vector<point>& cur_used, const std::vector<point>& cur_not_used, int src_id) const
		{
			return al.has_one_isolated_point(cur_used, cur_not_used, src_id);
		}

		const algo& al;

		greedy_detail_t(const algo& al) : al(al) {}

	} greedy_detail;

	//! subroutine which returns whether at least one point is isolated
	bool has_one_isolated_point(const std::vector<point>& used_points,
		const std::vector<point> &not_used_points,
		int src_id) const;

	//! subroutine which check whether there's one scc of movables
	bool one_scc_of_movables(const std::vector<point> &prev_used, const std::set<point> &new_area, const backed_up_grid &bug) const;

public:
	//! ctor taking a string containing the ca equation
	algo(const char* equation, cell_t border, bool dump_on_exit);

	//! ctor taking an istream to a ca table file
	algo(std::istream& is, cell_t border, bool dump_on_exit);

	void run(std::ostream& out);
};

}

#endif // GREEDY_H
