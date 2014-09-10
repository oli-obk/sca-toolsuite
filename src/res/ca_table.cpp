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

// TODO: rename this file: ca_table.cpp
#include "ca_table.h"

namespace sca { namespace ca {

constexpr const uint32_t tbl_detail::version_t::id;

tbl_detail::size_check::size_check(int size)
{
	if(size < 0)
	 throw "Error: Size negative (did you set num_states == 0?).";
	if(size > (1<<18))
	 throw "Error: This ca is too large for a table.";
}

tbl_detail::header_t::header_t(std::istream &stream)
{
	char buf[9];
	buf[8] = 0;
	stream.read(buf, 8);
	if(strcmp(buf, "ca_table"))
	 throw "Error: This file has no ca_table header.";
}

tbl_detail::version_t::version_t(const uint32_t &i)
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

unsigned _table_hdr_t::fetch_8(std::istream &stream)
{
	uint8_t res;
	stream.read((char*)&res, 1);
	return res;
}

_table_hdr_t::n_t _table_hdr_t::fetch_n(std::istream &stream) const
{
	std::vector<point> v(fetch_32(stream));
	std::cout << "fetching n: " << v.size() << std::endl;
	for(std::size_t i = 0; i < v.size(); ++i)
	{
		coord_t x = fetch_8(stream);
		coord_t y = fetch_8(stream);
		v[i] = point(x, y);
	}
	return n_t(std::move(v));
}

void _table_hdr_t::put_n(std::ostream &stream, const n_t& n) const
{
	uint32_t tmp = n.size();
	stream.write((char*)&tmp, 4);
	const auto dump_point =
		[&](const point& p) {
		char x = p.x;
		stream.write(&x, 1);
		char y = p.y;
		stream.write(&y, 1);
	};
	n.for_each(point::zero(), dump_point);
}

unsigned _table_hdr_t::fetch_32(std::istream &stream)
{
	uint32_t res;
	stream.read((char*)&res, 4);
	//	const u_coord_t res_u = res; // TODO: ???
	//	(void) res_u;
	//	std::cout << "READ: " << res << std::endl;
	return res;
}

void _table_hdr_t::dump(std::ostream &stream) const
{
	tbl_detail::header_t::dump(stream);
	tbl_detail::version_t::dump(stream);
//	uint32_t tmp = _n_in.size();
//	stream.write((char*)&tmp, 4);
	uint32_t tmp = own_num_states;
	stream.write((char*)&tmp, 4);
	put_n(stream, _n_in);
	put_n(stream, _n_out);
}

_table_hdr_t::_table_hdr_t(std::istream &stream) :
	header(stream),
	version(fetch_32(stream)),
	//	n_w(fetch_32(stream)),
	own_num_states(fetch_32(stream)),
	size_each((unsigned)ceil(log(own_num_states))), // TODO: use int arithm
	//	center((n_w - 1)>>1, (n_w - 1)>>1),
	_n_in(fetch_n(stream)),
	_n_out(fetch_n(stream)),
	center(_n_in.get_center_cell()),
	center_out(_n_out.get_center_cell()), // TODO: correct?
	bw(_n_in.get_max_w())
{
	std::cerr << "Number of states:" << own_num_states << std::endl;
	std::cerr << "N in: " << _n_in << std::endl;
	std::cerr << "N out: " << _n_out << std::endl;
}

// TODO: pass n via && ?
_table_hdr_t::_table_hdr_t(_table_hdr_t::cell_t num_states, const n_t& _n_in, const n_t& _n_out) :
	//	n_w((base::calc_border_width()<<1) + 1),
	own_num_states(num_states),
	size_each((unsigned)ceil(log(own_num_states))), // TODO: use int arithm
	_n_in(_n_in),
	_n_out(_n_out),
	center(_n_in.get_center_cell()),
	//	base::calc_border_width<bitgrid_traits>()), // TODO: don't calc bw 3 times...
	center_out(_n_out.get_center_cell()), // TODO: correct?
	bw(_n_in.get_max_w())
{
	std::cerr << "Number of states:" << own_num_states << std::endl;
	std::cerr << "N in: " << _n_in << std::endl;
	std::cerr << "N out: " << _n_out << std::endl;
}

} }

