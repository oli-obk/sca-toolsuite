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
#include <unistd.h>
#include <string>
#include <exception>
#include "general.h"

const point point::zero = point(0, 0);
const matrix matrix::id = matrix(1, 0, 0, 1);

void os_sleep(unsigned int seconds) { sleep(seconds); }
void os_clear() { system("clear"); }

int Program::run(int _argc, char **_argv, const HelpStruct *_help)
{
	argc = _argc; argv = _argv; help = _help;

	assert(help->description && help->syntax);
	if(argc > 1 && (!strcmp(argv[1],"--help")||!strcmp(argv[1],"-help")))
		{
			help->print_help();
			::exit(0);
		}

	int return_value = 0;
	try {
		return_value = main();
	} catch(const char* str) {
		exit(str);
	} catch(const std::string& str) {
		exit(str.c_str());
	} catch(std::exception e) {
		exit(e.what());
	} catch(...) {
		exit("Unknown error caught. This should never happen.");
	}
	return return_value;
}
