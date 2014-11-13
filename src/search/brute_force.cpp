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

#include "brute_force.h"

namespace brute
{
	int _stack_data::id_counter = 0;

	std::ostream& operator<<(std::ostream &stream, const pseudo_int &i)
	{
		return stream << bitgrid_t(2/*TODO*/,
			_dimension<coord_traits<int64_t>>(8 /*TODO*/, 1), 0, i.u);
	}

	io::serializer& operator<<(io::serializer &s, const brute_forcer &a)
	{
		int32_t version = 1;
		s << version;
		s << a.name << a.rgb32;
		s << a.orig_grid_unchanged;
		s << a.results;
		const brute_forcer::m_dep_graph_t& g = a.total_dep_graph;
		s << (std::size_t)g.num_edges();
		for(auto eitr = g.edges().begin(); eitr != g.edges().end(); ++eitr)
		{
			s << eitr.source() << eitr.target();
		}
		s << a.res_graph;
		s << a.stats.make_stat_results();
		return s;
	}

	std::ostream &brute_forcer::print(std::ostream &stream) const
	{
		return stream << mk_print(results);
	}

	brute_forcer::brute_forcer(const char *equation,
		types::cell_t border, bool dump_on_exit) :
		base(equation, border),
		n_in(ca.calc_n_in<char_traits>()),
		n_out(ca.calc_n_out<char_traits>()),
		readers_of(n_in.inverted() * n_out),
		writers_to(n_out.inverted() * n_in),
#ifdef DELETE_UNUSED_VERTS
		stats(false),
#else
		stats(true),
#endif
		debug_graph(dump_on_exit),
		dep_graph("dep_graph_", readers_of),
		total_dep_graph("dep_graph", readers_of)
	{
	}

	brute_forcer::brute_forcer(std::istream &is,
		types::cell_t border, bool dump_on_exit) :
		base(is, border),
		n_in(ca.calc_n_in<char_traits>()),
		n_out(ca.calc_n_out<char_traits>()),
		readers_of(n_in.inverted() * n_out),
		writers_to(n_out.inverted() * n_in),
#ifdef DELETE_UNUSED_VERTS
		stats(false),
#else
		stats(true),
#endif
		debug_graph(dump_on_exit),
		dep_graph("dep_graph_", readers_of),
		total_dep_graph("dep_graph", readers_of)
	{
	}

}
