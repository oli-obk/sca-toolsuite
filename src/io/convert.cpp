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

#include "general.h"
#include "io.h"

/*template<typename function>
call_function(const char* key, FILE* fp, std::vector<int>* grid, dimension* dim, int border_width = 1)
{
	if(!strcmp("rotors",key))
	 function<default_serializer>(fp, grid, drim, border_width);
	else
	 function<arrow_serializer>(fp, grid, drim, border_width);
}*/

#include <cstring>
#include <iostream>

bool file_ending_is(const char* filename, const char* ending)
{
	int end_len = strlen(ending);
	const char* fn_end = filename + strlen(filename);
	return (*(fn_end-end_len-1))=='.' && !strcmp(fn_end-end_len,ending);
}


#if 0
// TODO: use variadic templates instead
template<typename this_type, typename next_type>
struct typelist
{
	typedef this_type type;
	typedef next_type next;
};

template<typename t>
bool try_this(const char* name, default_grid& grid)
{
	typedef typename t::type _type;
	if(!strcmp(_type::name(), name)) {
		const _type infile(stdin);
		converter<_type, default_grid> c(infile, grid);
		c.convert();
		return true;
	}
	else
	 return try_this<typename t::next>(name, grid);
}

struct end_of_list {};
template<>
bool try_this<end_of_list>(const char* name, default_grid& grid)
{
	return false;
}

template<typename t>
bool try_this2(const char* name, const_default_grid& grid)
{
	typedef typename t::type _type;
	if(!strcmp(_type::name(), name)) {
		_type outfile(stdout);
		converter<const_default_grid, _type> c(grid, outfile);
		c.convert();
		return true;
	}
	else
	 return try_this2<typename t::next>(name, grid);
}

template<>
bool try_this2<end_of_list>(const char* name, const_default_grid& grid)
{
	return false;
}


typedef typelist<FileGrid, typelist<FileArrowGrid , end_of_list> > file_list;
//typedef end_of_list file_list;

#endif

class MyProgram : public Program
{



	//static const fmapping function_names[3];

	inline bool valid_function_name(const char* function_name) {
/*		const fmapping* fmap;
		for(fmap = function_names;
			fmap->name && strcmp(function_name,fmap->name); fmap++) ;
		return (fmap->name != NULL);*/
	//	return false;
		return true;
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

	/*	const fmapping* fmap;
		for(fmap = function_names;
			fmap->name && strcmp(scanfunc_name,fmap->name); fmap++) ;
		scanfunc = fmap->name ? fmap->scanfunc : function_names[0].scanfunc;

		for(fmap = function_names;
			fmap->name && strcmp(printfunc_name,fmap->name); fmap++) ;
		printfunc = fmap->name ? fmap->printfunc : function_names[0].printfunc;
*/
		std::vector<int> _vector;
		dimension _dim;


#if 0
		default_grid grid(_grid(_vector, _dim, 1));
		try_this<file_list>(scanfunc_name, grid);
		const_default_grid grid2(_const_grid(grid.grid.grid, grid.grid.dim, grid.grid.border_width));
		try_this2<file_list>(printfunc_name, grid2);
#endif

	/*	read_array(stdin, &grid, &dim, scanfunc);
		write_array(stdout, &grid, &dim, printfunc);*/
	//	call_function(_read_array, stdin, &grid, &dim, scanfunc);

		exit("Not working in this commit.");
		return 0;
	}
};


/*

const MyProgram::fmapping MyProgram::function_names[3] = {
//	{ "numbers", &read_number, &write_number },
//	{ "rotors", &read_arrow, &write_arrow },
	{ NULL, NULL, NULL } // sentinel
};*/

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



