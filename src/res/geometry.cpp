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

#include <algorithm>

#include "geometry.h"

const point point::zero = point(0, 0);
const matrix matrix::id = matrix(1, 0, 0, 1);
constexpr point rect_storage_origin::ul;

void grid_t::resize_borders(u_coord_t new_border_width)
{
	if(new_border_width != bw)
	{
		const u_coord_t nbw = new_border_width,
			obw = bw;

		u_coord_t old_lw = _dim.width();
		u_coord_t new_bw2 = nbw << 1;
		dimension hd = human_dim();

		_dim = dimension(hd.width() + new_bw2, hd.height() + new_bw2);
		bw = nbw;

		u_coord_t new_lw = _dim.width();
		u_coord_t human_lw = hd.width();
		area_t old_top = obw * (old_lw + 1),
			new_top = nbw * (new_lw + 1);
		area_t old_bot = old_top + old_lw * (hd.height() - 1),
			new_bot = new_top + new_lw * (hd.height() - 1);

		auto for_loop
			= [&](std::size_t n, std::size_t o) {
		//		std::cout << "copying " << human_lw << " bytes from " << o << " to " << n << std::endl;
				std::copy_n(_data.begin() + o, human_lw, /*result: */ _data.begin() + n);
		//		std::cout << "filling " << new_bw2 << " bytes at " << (n + human_lw) << std::endl;
				std::fill_n(_data.begin() + n + human_lw, new_bw2, INT_MIN);
		};

		if(nbw < obw)
		// move elements backward
		// so start with the top line
		{
			// top border line is correct already

			for( std::size_t n = new_top, o = old_top;
				n <= new_bot;
				n += new_lw, o+= old_lw)
			 for_loop(n, o);

			std::fill(_data.begin() + new_bot + new_lw,
				_data.begin() + old_bot + old_lw, INT_MIN);

			_data.resize(_dim.area());
		}
		else if(nbw > obw)
		// move elements forward
		// so start with the bottom line
		{
			_data.resize(_dim.area(), INT_MIN);

			// top border line is filled by resize()

			for(std::size_t n = new_bot, o = old_bot;
				n >= new_top; n -= new_lw, o-= old_lw)
			 for_loop(n, o);

			std::fill(_data.begin() + old_top,
				_data.begin() + new_top, INT_MIN);
		}

	/*	std::cout << "grid now:" << std::endl;
		for(const cell_t& c : data)
		{
			std::cout << c << std::endl;
		}*/
	}
}

void grid_t::insert_stripe_vert(u_coord_t pos, u_coord_t ins_len)
{
	// update vector, keep _dim
	// we can use _dim, since we always need the full h/w
	u_coord_t ht = _dim.height();
	u_coord_t mv_amt = ht * ins_len;
	coord_t mv_start = index_internal(
				point(bw + pos, ht-1));

	_data.resize(_data.size() + mv_amt, INT_MIN);

	// lowest row separately, since it has a different "last"
	// TODO: what if the grid has 0 lines?
	{
		auto first = _data.begin() + mv_start;
		auto last = _data.end() - mv_amt;
		std::copy_backward(first, last, _data.end());

		mv_start -= _dim.width();
		mv_amt -= ins_len;
	}

	for( ; mv_start >= 0;
		mv_start -= _dim.width(), mv_amt -= ins_len)
	{
		auto first = _data.begin() + mv_start;
		auto last = first + _dim.width();
		std::copy_backward(first, last, last + mv_amt);
	}

	// finally, update _dim
	_dim = dimension(_dim.width() + ins_len, ht);
}

void grid_t::insert_stripe_hor(u_coord_t pos, u_coord_t ins_len)
{
	u_coord_t mv_amt = _dim.width() * ins_len;
	std::cout << "inserting hor. stripe of size "
		<< ins_len << std::endl;

	_data.resize(_data.size() + mv_amt);

	auto first = _data.begin() + index_internal(
				point(0, bw + pos));
	std::copy_backward(first, _data.end() - mv_amt, _data.end());

	_dim = dimension(_dim.width(), _dim.height() + ins_len);
}
