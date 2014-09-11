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

#include "simulate.h"
#include "general.h"
#include "io.h"
#include "ca_convert.h"

using namespace sca;

// TODO: own sim type class, inherit
class MyProgram : public Program, sim::ulator
{
	exit_t main()
	{
		bool rot = false;
		bool mirr = false;

		switch(argc)
		{
			case 3:
				mirr = atoi(argv[2]);
			case 2:
				rot = atoi(argv[1]);
			case 1:
				break;
			default:
				exit_usage();
		}

		ca::converter<ca::type::grids, ca::type::table> c;
		c(std::cin, std::cout, rot, mirr);

		return exit_t::success;
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



