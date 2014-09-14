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

class MyProgram : public Program
{
	exit_t main()
	{
		const char *in_name = "unspecified", *out_name = in_name;

		if(argc > 1 && !strcmp(argv[1], "help"))
		{
			std::cerr << "Available formats for ca converter:"
				<< std::endl;
			ca::name_type_map.dump_names(std::cerr);
			return exit_t::success;
		}

		switch(argc)
		{
			case 3: out_name = argv[2];
			case 2: in_name = argv[1];
			case 1:
				break;
			default:
				exit_usage();
		}

		ca::convert_dynamic(in_name, out_name);
		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "ca/converter <in-format> <out-format>"
		"";
	help.description = "Makes a copy of a stored cellular automaton\n"
		"Type `ca/converter help' for available formats";
	help.add_param("<in-format>", "input format or `'");
	help.input = "the known, stored ca in a valid format";
	help.output = "the target for the copy";

	MyProgram p;
	return p.run(argc, argv, &help);
}



