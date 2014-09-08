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

#include "ca.h"

namespace sca { namespace ca {

constexpr const uint32_t _table_hdr_t::version_t::id;



_table_hdr_t::size_check::size_check(int size)
{
	if(size < 0)
	 throw "Error: Size negative (did you set num_states == 0?).";
	if(size > (1<<18))
	 throw "Error: This ca is too large for a table.";
}

_table_hdr_t::header_t::header_t(std::istream &stream)
{
	char buf[9];
	buf[8] = 0;
	stream.read(buf, 8);
	if(strcmp(buf, "ca_table"))
	 throw "Error: This file has no ca_table header.";
}

_table_hdr_t::version_t::version_t(const uint32_t &i)
{
	if(i != id)
	{
		//char err_str[] = "Incompatible versions:\n" // 23
		//	"file: ................,\n" // 6 + 16 + 2
		//	"expected: ................";
		//sprintf(err_str + 23 + 6, "%016d", i);
		//sprintf(err_str + 23 + 6 + 16 + 2 + 10, "%016d", id);
		throw std::string("Incompatible versions: "
				  "read: " + std::to_string(i) +
				  ", expected: " + std::to_string(id) + ".");
	}
}

void _table_hdr_t::dump(std::ostream &stream) const
{
	header_t::dump(stream);
	version_t::dump(stream);
	uint32_t tmp = n_w;
	stream.write((char*)&tmp, 4);
	tmp = own_num_states;
	stream.write((char*)&tmp, 4);
}

_table_hdr_t::_table_hdr_t(const char *equation, _table_hdr_t::cell_t num_states) :
	base(equation, num_states),
	n_w((base::calc_border_width()<<1) + 1),
	own_num_states(base::num_states),
	size_each((unsigned)ceil(log(own_num_states))), // TODO: use int arithm
	center(base::calc_border_width(), base::calc_border_width()), // TODO: don't calc bw 3 times...
	bw(compute_bw()),
	neighbourhood(compute_neighbourhood())
{
}

_table_hdr_t::_table_hdr_t(std::istream &stream) :
	base("v", 0), // not reliable
	header(stream),
	version(fetch_32(stream)),
	n_w(fetch_32(stream)),
	own_num_states(fetch_32(stream)),
	size_each((unsigned)ceil(log(own_num_states))), // TODO: use int arithm
	center((n_w - 1)>>1, (n_w - 1)>>1),
	bw(compute_bw()),
	neighbourhood(compute_neighbourhood())
{
}

} }
