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

#include <cstring>
#include "general.h"
#include "io.h"
#include "ca_convert.h" // TODO: -> cmake deps

using namespace sca;

// TODO: own symm type class, inherit
class MyProgram : public Program
{
// symmetry not implemented:
#if USE_SYMMETRY
	enum class symmetry_type
	{
		none,
		rotate,
		rotate_mirror
	};

	struct symm_wrapper
	{
		symmetry_type t;
		const char* str;
	};

	const symm_wrapper wraps[4] = // TODO: why is 4 needed?
	{
		{ symmetry_type::none, "none" },
		{ symmetry_type::rotate, "rotate" },
		{ symmetry_type::rotate_mirror, "rotate+mirror" },
	};

	symmetry_type type_by_str(const char* str) const
	{
		for(const symm_wrapper& i : wraps)
		 if(!strcmp(i.str, str))
		  return i.t;
		exitf("Invalid symmetry type %s", str);
		return symmetry_type::none; // suppress compiler error
	}
#endif

	exit_t main()
	{
		//int number_of_states;
		//symmetry_type symm_type = symmetry_type::none;
		/*switch(argc)
		{
			case 3:
				symm_type = type_by_str(argv[2]);
			case 2:
				number_of_states = atoi(argv[1]); break;
			default:
				exit_usage();
		}*/
		assert_usage(argc == 1);

		using namespace sca::ca;
		converter<type::grids, type::formula> c;
		c();

		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "ca/transf_by_grids";
	//help.syntax = "ca/transf_by_grids <number of states>";
	help.description = "Builds a transition formula by transition grids";
	help.input = "the transition grids, in some special format."
		"The first grid marks the changing cell (0),"
		"the changing cell in the neighborhood (1),"
		"its neighborhoud (2) and dontcares (>2)."
		"Then, there follows a sequence of <input grid> <output grid>"
		"where input grid gives the neighborhood.";
	// TODO: describe?
	help.output = "the formula, as `ca/ca' can read it";
//	help.add_param("number of states", "the ca's number of states");

	MyProgram p;
	return p.run(argc, argv, &help);
}

