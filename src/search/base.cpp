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

#include "base.h"

bool base::global_abort = false;

base::base(const char *equation, types::cell_t border) :
	ca(equation, 3),
	ca_n(ca.n_in()),
	ca_n_2(ca_n * ca_n),
	dead_state(border)
{
	//std::cout << "Neighbourhood: " << ca_n << std::endl;
}


base::base(std::istream &is, types::cell_t border) :
	ca(is),
	ca_n(ca.n_in()), // TODO: forbid ca_n and ca_n_2
	ca_n_2(ca_n * ca_n),
	dead_state(border)
{
	//std::cout << "Neighbourhood: " << ca_n << std::endl;
}

void base::parse() {
	_parse();
	init();
	std::cerr << "IO complete" << std::endl << std::endl;
}

void base::preprocess_results()
{
}

std::ostream& operator<<(std::ostream &stream, const base &b)
{
	return b.print(stream);
}
