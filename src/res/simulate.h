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

#ifndef SIMULATE_H
#define SIMULATE_H

namespace sca {
namespace sim {

class ulator
{
protected:
	enum class sim_type
	{
		end,
		role,
		more,
		anim,
		undefined
	};
	sim_type type_by_str(const char* str);
private:
	struct sim_wrapper
	{
		sim_type t;
		const char* str;
	};

	static sim_wrapper wraps[4]; // TODO: why is 4 needed?
};

}
}

#endif // SIMULATE_H
