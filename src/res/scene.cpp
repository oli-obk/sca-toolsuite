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

class MyProgram : public Program
{
	exit_code main()
	{


		/*
		 * args
		 */

		switch(argc)
		{
		//	case 2:
		//		tbl_file = argv[1];
		//		break;
			case 1: break;
			default:
				exit_usage();
		}

		/*
		 * parsing
		 */



		return exit_code::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "ca/scene [<scene-file>]"
		"";
	help.description = "Converts a scene into a ca document file."
		"";
	help.input = "Input grid in a special format.";
	help.output = "The ca document";
//	help.add_param("dump|nodump", "whether to dump graph on exit/abort");

	MyProgram p;
	return p.run(argc, argv, &help);
}
