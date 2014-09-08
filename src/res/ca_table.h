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

#ifndef CA_TABLE_H
#define CA_TABLE_H

#include "ca_eqs.h"
#include "bitgrid.h"

namespace sca { namespace ca {

//! header structure and basic utils for a table file
class _table_hdr_t : public _eqsolver_t<
	bitgrid_traits, bitgrid_cell_traits> // TODO: make base a template?
{
protected:
	using base = _eqsolver_t<
		bitgrid_traits, bitgrid_cell_traits>;

	using u_coord_t = typename bitgrid_traits::u_coord_t;
	using cell_t = typename bitgrid_cell_traits::cell_t;

	using coord_t = typename bitgrid_traits::coord_t;
	using point = _point<bitgrid_traits>;
	using dimension = _dimension<bitgrid_traits>;
public:
	using n_t = _n_t<bitgrid_traits, std::vector<point>>;

	/*
		table format:
		header: ca_table (8 char)
		version: 1 (uint_32)
		nw (uint_32)
		own_num_states (uint_32)
		table (size_each * n_w * n_w * uint_64)
	*/

protected:
	class size_check
	{
		size_check(int size);
	};

	struct header_t
	{
		header_t(std::istream& stream);
		static void dump(std::ostream &stream)
		{
			stream.write("ca_table", 8);
		}
		header_t() {}
	};

	const header_t header;

	struct version_t
	{
		static constexpr const uint32_t id = 1;
		version_t(const uint32_t& i);
		version_t() {}
		static void dump(std::ostream &stream)
		{
			stream.write((char*)&id, 4);
		}
	};
	const version_t version;

	const u_coord_t n_w; // TODO: u_coord_t
	const unsigned own_num_states; //!< number of possible states per cell
	const u_coord_t size_each; // TODO: u_coord_t
	const point center;

	// data for outside:
	const u_coord_t bw; // TODO: u_coord_t
	const n_t neighbourhood;

	u_coord_t compute_bw() const noexcept
	{
		return (n_w - 1)>>1;
	}

	n_t compute_neighbourhood() const noexcept
	{
		u_coord_t bw = border_width();
		u_coord_t n_width = (bw<<1) + 1;
		dimension moore = { n_width, n_width };
		return n_t(moore, point(bw, bw));
	}
public:
	unsigned border_width() const noexcept { return bw; } // TODO: should ret reference
	const n_t& n_in() const noexcept { return neighbourhood; }
protected:
	static unsigned fetch_32(std::istream& stream)
	{
		uint32_t res;
		stream.read((char*)&res, 4);
	//	const u_coord_t res_u = res; // TODO: ???
	//	(void) res_u;
	//	std::cout << "READ: " << res << std::endl;
		return res;
	}

	//! O(1)
	void dump(std::ostream& stream) const;

	_table_hdr_t(std::istream& stream);

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_table_hdr_t(const char* equation, cell_t num_states = 0);
};

template<template<class ...> class TblCont>
class _table_t : public _table_hdr_t // TODO: only for reading?
{
private:
	// TODO!!! table can use uint8_t in many cases!
	const TblCont<uint64_t> table;

	using storage_t = uint64_t;

private:
	static TblCont<uint64_t> fetch_tbl(std::istream& stream, unsigned size_each, unsigned n_w)
	{
		TblCont<uint64_t> res(1 << (size_each * n_w * n_w));
		stream.read((char*)res.data(), res.size() * 8);
		return res;
	}

	// TODO : static?
	TblCont<uint64_t> calculate_table() const
	{
		TblCont<uint64_t> tbl;
	//	tbl.reserve(1 << (size_each * n_w * n_w)); // ctor can not reserve
		tbl.resize(1 << (size_each * n_w * n_w));

		bitgrid_t grid(size_each, dimension(n_w, n_w), 0, 0);
		const dimension& dim = grid.internal_dim();
		const std::size_t max = (int)pow(num_states, (n_w * n_w));

		std::size_t percent = 0, cur;

		std::cerr << "Precalculating table, please wait..." << std::endl;
		// odometer
//		int last_val = -1;
		for(std::size_t i = 0; i < max; ++i)
		{
			// evaluate
			tbl.at(grid.raw_value()) = (base::
				calculate_next_state(grid.raw_value(), size_each, center, dim));

#ifdef SCA_DEBUG
//			if(tbl.at(grid.raw_value()) != grid[center])
//			 std::cerr << grid << " => " << tbl.at(grid.raw_value()) << std::endl;
#endif

			cur = (i * 100) / max; // max can never be 0
			if(percent < cur)
			{
				percent = cur;
				std::cerr << "..." << percent << " percent" << std::endl;
			}
//			if((last_val != -1) && (last_val != (int)grid.raw_value() - 1)) exit(1);
//last_val = grid.raw_value();
			// increase
			{
				bool go_on = true;
				for(bitcell_itr itr = grid.begin(); itr != grid.end() && go_on; ++itr)
				{
					go_on = ((*itr = (((*itr) + 1) % num_states)) == 0);
				}
			}
		}

		return tbl;
	}

public:
	//! O(table)
	void dump(std::ostream& stream) const
	{
		_table_hdr_t::dump(stream);
		stream.write((char*)table.data(), table.size() * 8);
	}

	_table_t(std::istream& stream) :
		_table_hdr_t(stream),
		table(fetch_tbl(stream, size_each, n_w))
	{
	}

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_table_t(const char* equation, cell_t num_states = 0) :
		_table_hdr_t(equation, num_states),
		table(calculate_table())
	{
	}

	//! Note: the type traits here can be different than ours
	//! Runtime: O(N)
	template<class GT, class GCT>
	int calculate_next_state(const typename GCT::cell_t* const cell_ptr,
		const _point<GT>& p, const _dimension<GT>& dim) const
	{
		// TODO: class member?
		using grid_cell_t = typename GCT::cell_t;

		(void)p; // for a ca, the coordinates are no cell input
		bitgrid_t bitgrid(size_each, dimension(n_w, n_w), 0);

		grid_cell_t min = std::numeric_limits<grid_cell_t>::max(),
			max = std::numeric_limits<grid_cell_t>::min(); // any better alternative?

		for(const point& p2 : bitgrid.points())
		{
			const point offs = p2 - center;
			const grid_cell_t* const ptr = cell_ptr + (grid_cell_t)(offs.y * dim.width() + offs.x);
			bitgrid[p2] = *ptr;
			min = std::min(min, *ptr);
			max = std::max(max, *ptr);
		}

		// todo: better hashing function for not exactly n bits?
		const bool in_range = (min >= 0 && max < (int)own_num_states);
		return in_range
			? table[bitgrid.raw_value()]
			: *cell_ptr; // can not happen, except for border -> don't change
	}

};

template<class T>
class table_vector
{
	std::vector<T>& vec;
	using reference = typename std::vector<T>::reference;
	using pointer = typename std::vector<T>::pointer;
	using value_type = typename std::vector<T>::value_type;
public:
	table_vector(const std::size_t& sz) : vec(sz) {}
	void reserve(const std::size_t& sz) { vec.reserve(sz); }
	void push_back(value_type&& sz) { vec.push_back(sz); }
	void dump(const std::ostream& stream) const {
		stream.write((char*)vec.data(), vec.size() * 8); }
};

using table_t = _table_t<std::vector>;

}}

#endif // CA_TABLE_H
