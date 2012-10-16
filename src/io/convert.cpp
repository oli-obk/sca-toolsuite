/*************************************************************************/
/* sca toolsuite - a toolsuite to simulate sandpile cellular automata.   */
/* Copyright (C) 2011-2012                                               */
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
#include "io.h"

class MyProgram : public Program
{
	struct fmapping {
		const char* name;
		bool (*scanfunc)(FILE*, int*);
		void (*printfunc)(FILE*, int);
	};

	static const fmapping function_names[3];

	inline bool valid_function_name(const char* function_name) {
		const fmapping* fmap;
		for(fmap = function_names;
			fmap->name && strcmp(function_name,fmap->name); fmap++) ;
		return (fmap->name != NULL);
	}

	int main()
	{
		bool (*scanfunc)(FILE*, int*);
		void (*printfunc)(FILE*, int);
		const char* scanfunc_name = "default";
		const char* printfunc_name = "default";
		switch(argc)
		{
			case 3:
				printfunc_name = argv[2];
				assert_usage(valid_function_name(printfunc_name));
			case 2:
				scanfunc_name = argv[1];
				assert_usage(valid_function_name(scanfunc_name));
			case 1: break;
			default:
				exit_usage();
		}

		const fmapping* fmap;
		for(fmap = function_names;
			fmap->name && strcmp(scanfunc_name,fmap->name); fmap++) ;
		scanfunc = fmap->name ? fmap->scanfunc : function_names[0].scanfunc;

		for(fmap = function_names;
			fmap->name && strcmp(printfunc_name,fmap->name); fmap++) ;
		printfunc = fmap->name ? fmap->printfunc : function_names[0].printfunc;

		std::vector<int> grid;
		dimension dim;

	/*	read_array(stdin, &grid, &dim, scanfunc);
		write_array(stdout, &grid, &dim, printfunc);*/
		exit("Not working in this commit.");
		return 0;
	}
};

const MyProgram::fmapping MyProgram::function_names[3] = {
	{ "numbers", &read_number, &write_number },
	{ "rotors", &read_arrow, &write_arrow },
	{ NULL, NULL, NULL } // sentinel
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "convert [<in-format> [<out-format>]]";
	help.description = "Converts different types of grids from stdin to stdout\n"
		"Does *not* detect invalid formats, it is up to the user\n"
		"Supported formats: numbers, rotors";
	help.input = "input grid of type <in-format>";
	help.output = "output grid of type <out-format>";
	help.add_param("<in-format>", "input format, defaults to `numbers'");
	help.add_param("<out-format>", "output format, defaults to `numbers'");

	MyProgram p;
	return p.run(argc, argv, &help);
}



