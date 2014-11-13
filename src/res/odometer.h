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

#ifndef ODOMETER_H
#define ODOMETER_H

#include <vector>

// adapted from [1]
template<bool AllowEmpty = false, class Container, class Functor>
void odometer(const Container& vectors, const Functor& functor)
{
	const int K = vectors.size();

	if ( K > 0 )
	{
		using const_iterator = typename Container::value_type::const_iterator;
		std::vector<const_iterator> itrs(K);

		const auto incr_iter = [&](const std::size_t& id){
			if(!AllowEmpty || vectors[id].size())
			 ++itrs[id];
		};

		std::size_t size = 1;
		{
			int j = 0;
			// initialize itrs and check for emptiness
			for( auto& cur_it : itrs) {
				if(vectors[j].size() || !AllowEmpty)
				 size *= vectors[j].size();
				cur_it = vectors[j++].begin();
			}
		}

		if(AllowEmpty || size)
		for(std::size_t j = 0; j < size; ++j)
		{
			// process the pointed-to elements :
			functor(itrs); // (TODO:) make abort possibility by return value

			// increment odometer
			incr_iter(K-1);
			for (std::size_t i = K-1; (i > 0) && (itrs[i] == vectors[i].end()); --i) {
				itrs[i] = vectors[i].begin();
				incr_iter(i-1);
			}
		}
	}
}

// sources:
// [1] http://stackoverflow.com/???
//     /I don't recall the url (TODO)

#endif // ODOMETER_H



