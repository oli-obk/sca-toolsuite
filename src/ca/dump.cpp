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

#include "general.h"
#include "io.h"
#include "ca.h"

using namespace sca;

// TODO: own sim type class, inherit
class MyProgram : public Program
{
	enum class sim_type
	{
		end,
		role,
		more,
		anim,
		undefined
	};

	struct sim_wrapper
	{
		sim_type t;
		const char* str;
	};

	sim_wrapper wraps[4] = // TODO: why is 4 needed?
	{
		{ sim_type::end, "end" },
		{ sim_type::role, "role" },
		{ sim_type::more, "more" },
		{ sim_type::anim, "anim" }
	};

	sim_type type_by_str(const char* str)
	{
		sim_type sim = sim_type::undefined;
		for(sim_wrapper& i : wraps)
		 if(!strcmp(i.str, str))
		  sim = i.t;
		return sim;
	}

	int main()
	{
		switch(argc)
		{
			case 1:
				break;
			default:
				exit_usage();
		}

		std::istreambuf_iterator<char> eos;
		std::string s(std::istreambuf_iterator<char>(std::cin), eos);

		ca::ca_table_t tbl(s.c_str(), 3); // TODO: 3
		tbl.dump(std::cout);

		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "ca/dump"
		"";
	help.description = "Dumps a cellular automaton (ca).\n"
		"The number of states is always 3 (0-2).\n"
		"This can be changed in the future.";
	help.input = "equation string, describing the local tf";
	help.output = "table for the local tf";

	MyProgram p;
	return p.run(argc, argv, &help);
}


