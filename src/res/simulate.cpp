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
#include "simulate.h"

namespace sca {
namespace sim {

ulator::sim_wrapper ulator::wraps[4]
{
	{ sim_type::end, "end" },
	{ sim_type::role, "role" },
	{ sim_type::more, "more" },
	{ sim_type::anim, "anim" }
};

ulator::sim_type ulator::type_by_str(const char *str)
{
	sim_type sim = sim_type::undefined;
	for(sim_wrapper& i : wraps)
	if(!strcmp(i.str, str))
	 sim = i.t;
	return sim;
}

}
}

