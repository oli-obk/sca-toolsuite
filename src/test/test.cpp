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
#include "grid.h"
#include "ca.h"

class MyProgram : public Program
{
	int main()
	{
		// TODO: re-enable?
#if 0
		sca::ca::conf_t c1({1, 5, 3});
		sca::ca::conf_t c2({4, 5, 6});
		std::set<point> s1{{0, 0}, {2, 0}, {4,0}};
		std::set<point> s2{{1, 0}, {2, 0}, {3,0}};
		sca::ca::conf_t c3 =
			sca::ca::conf_t::merge(s1, c1, s2, c2);
		std::cout << c3 << std::endl;
		assert_always(c3[0]==1, "0");
		assert_always(c3[1]==4, "1");
		assert_always(c3[2]==5, "2");
		assert_always(c3[3]==6, "3");
		assert_always(c3[4]==3, "4");
#endif
		//std::set<point> s4{{0, 0}, {2, 0}};
		//sca::ca::conf_t c5 = sca::ca::conf_t::substract(s1, c1, s4, )

		return 0;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "test/test";
	help.description = "A file only to test internal"
		"(often new) features.";

	MyProgram p;
	return p.run(argc, argv, &help);
}

