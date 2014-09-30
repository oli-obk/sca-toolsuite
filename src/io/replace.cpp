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

#include "io/secfile.h"
#include "grid.h"
#include "general.h"
#include "ca_convert.h"

/*
struct grid_pair_t
{
	grid_t in, out;
public:
	friend std::istream& operator>> (std::istream& stream,
		grid_pair_t& gp) {
		return stream >> gp.in >> gp.out;
	}
};*/
using grid_pair_t = sca::grid_io::grid_pair_t; // TODO: abuse


class rules_t : public sca::io::supersection_t
{
public:
	rules_t() : supersection_t(type_t::batch)
	{
		init_factory<sca::io::leaf_template_t<grid_pair_t>>();
		set_batch_str("replace");
	}
};



class MyProgram : public Program
{


	exit_t main()
	{
		grid_t g_in(std::cin, 0);

		assert_usage(argc == 2);

		std::ifstream in(argv[1]);
		sca::io::secfile_t inf(in);
		rules_t rules;

		try {
			rules.parse(inf);
		} catch(sca::io::secfile_t::error_t ife) {
			std::cerr << "infile line " << ife.line << ": "	 << ife.msg << std::endl;
		}

		grid_t changed (g_in.human_dim(), 0, 0);

		for(const point& p : g_in.points())
		{
			for(std::size_t j = 0; j < rules.max(); ++j)
			{
				const grid_pair_t& gp = rules.value<grid_pair_t>(j);

				const rect scan_rect(point(0,0), point(g_in.human_dim().lr() - gp.in.human_dim().lr()));

				bool equal = scan_rect.contains(p);

				for(auto itr = gp.in.points().begin(); equal && itr != gp.in.points().end(); ++itr)
				 equal = equal && /*(!changed[p+*itr]) &&*/ (gp.in[*itr] == g_in[p + *itr]);

				if(equal)
				for(const point& p2 : gp.in.points())
				{
					g_in[p + p2] = gp.out[p2];
					changed[p + p2] = 1;
				}

			}

		}

		std::cout << g_in;

		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "io/replace <rule-file>";
	help.description = "\n"
		".";
	help.input = "input grid";
	help.output = "output grid";
	help.add_param("<rule-file>", "file with replacement rules");
	MyProgram p;
	return p.run(argc, argv, &help);
}


