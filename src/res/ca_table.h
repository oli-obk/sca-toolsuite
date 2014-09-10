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

namespace tbl_detail
{

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
	static constexpr const uint32_t id = 2;
	version_t(const uint32_t& i);
	version_t() {}
	static void dump(std::ostream &stream)
	{
		stream.write((char*)&id, 4);
	}
};

}

//! header structure and basic utils for a table file
class _table_hdr_t : public eqsolver_t // TODO: make base a template?
{
protected:
	using base = eqsolver_t;

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
		version: 2 (uint_32)
		own_num_states (uint_32)
		n_in size (uint_32)
		n_in (each 2xuint_8)
		n_out size
		n_out
		table (size_each * n_w * n_w * uint_64)
	*/

protected:

	const tbl_detail::header_t header;
	const tbl_detail::version_t version;

//	const u_coord_t n_w; // TODO: u_coord_t
	const unsigned own_num_states; //!< number of possible states per cell
	const u_coord_t size_each; // TODO: u_coord_t
	const n_t _n_in, _n_out;
	const point center;
	const point center_out;

	// data for outside:
	const u_coord_t bw; // TODO: u_coord_t

	static unsigned fetch_8(std::istream& stream);

	n_t fetch_n(std::istream& stream) const;
	void put_n(std::ostream &stream, const n_t &n) const;

public:
/*	unsigned border_width() const noexcept { return bw; } // TODO: should ret reference
	const n_t& n_in() const noexcept {
		std::set<n_t::>
	}
	const n_t& n_out() const noexcept {  }*/
protected:
	static unsigned fetch_32(std::istream& stream);

	//! O(1)
	void dump(std::ostream& stream) const;

	_table_hdr_t(std::istream& stream);

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_table_hdr_t(const char* equation, cell_t num_states = 0);

	template<class Traits>
	typename Traits::u_coord_t calc_border_width() const noexcept { return bw; }
	template<class Traits>
	_n_t_v<Traits> calc_n_in() const noexcept {
		using point = _point<Traits>;
		std::vector<point> res_v;
		for(const auto& p : _n_in)
		 res_v.push_back(point(p.x, p.y));
		return _n_t_v<Traits>(std::move(res_v));
	}
	template<class Traits>
	_n_t_v<Traits> calc_n_out() const noexcept {
		using point = _point<Traits>;
		std::vector<point> res_v;
		for(const auto& p : _n_out)
		 res_v.push_back(point(p.x, p.y));
		return _n_t_v<Traits>(std::move(res_v));
	}
};

template<template<class ...> class TblCont, class Traits, class CellTraits>
class _table_t : public _table_hdr_t // TODO: only for reading?
{
private:
	// TODO!!! table can use uint8_t in many cases!
	const TblCont<uint64_t> table;

	using storage_t = uint64_t;


	using b = _table_hdr_t; // TODO: remove b everywhere
	using typename b::cell_t;
	using typename b::point;

private:
	static TblCont<uint64_t> fetch_tbl(std::istream& stream, unsigned size_each, unsigned n_size)
	{
		TblCont<uint64_t> res(1 << (size_each * n_size));
		stream.read((char*)res.data(), res.size() * 8);
		return res;
	}

	// TODO : static?
	//! used to dump an in-memory-table from an equation
	TblCont<uint64_t> calculate_table() const
	{
		TblCont<uint64_t> tbl;
		tbl.resize(1 << (b::size_each * b::_n_in.size())); // ctor can not reserve

		const dimension n_in_dim(b::_n_in.get_dim().dx(), b::_n_in.get_dim().dy());
		bitgrid_t grid(b::size_each, n_in_dim, 0, 0);
		const std::size_t max = (int)pow(b::num_states, b::_n_in.size());

		std::size_t percent = 0, cur;

		std::cerr << "Precalculating table, please wait..." << std::endl;
		// odometer
//		int last_val = -1;
		bitgrid_t tbl_idx(b::size_each, dimension(_n_in.size(), 0), 0, 0);

		dimension n_out_dim = _n_out.get_dim();
		::grid_t tmp_result(::dimension(n_out_dim.dx(), n_out_dim.dy()), 0);
		bitgrid_t bit_tmp_result(b::size_each, n_out_dim, 0, 0);

		for(std::size_t i = 0; i < max; ++i)
		{
			// evaluate
			// TODO: tmp_result should be bitgrid.
			calculate_next_state_grids(grid.raw_value(), b::size_each,
				/*::point(b::center.x, b::center.y)*/ b::center,
				grid.internal_dim(),
				&tmp_result[::point(center_out.x, center_out.y)],
				::dimension(n_out_dim.dx(), n_out_dim.dy()));
			for(const point& p : _n_out)
			 bit_tmp_result[center_out + p] = tmp_result[::point(center_out.x + p.x, center_out.y + p.y)];
			tbl.at(tbl_idx.raw_value()) = bit_tmp_result.raw_value();

//			std::cerr << grid << " -> " << tmp_result << std::endl;

#ifdef SCA_DEBUG
//			if(tbl.at(grid.raw_value()) != grid[center])
//			 std::cerr << grid << " => " << tbl.at(grid.raw_value()) << std::endl;
#endif

			cur = (i * 100) / max; // max can never be 0 // TODO: 100 / max = const
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

				int digit = 0;
				for(auto itr = _n_in.cbegin(); go_on && (itr != _n_in.cend()); ++itr)
				{
					const point& p = *itr;
		//			std::cerr << "increasing: " << p << " = " << (center + p) << std::endl;

					bit_reference r = grid[center + p];
					go_on = ((r = ((r + 1) % b::num_states)) == 0);

					bit_reference r2 = tbl_idx[point(digit, 0)];
					r2 = ((r2 + 1) % b::num_states);

					++digit;
				}
			}
		}

		return tbl;
	}

public:
	//! O(table)
	void dump(std::ostream& stream) const
	{
		b::dump(stream);
		stream.write((char*)table.data(), table.size() * 8);
	}

	_table_t(std::istream& stream) :
		b(stream),
		table(fetch_tbl(stream, b::size_each, b::_n_in.size()))
	{
	}

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_table_t(const char* equation, cell_t num_states = 0) :
		b(equation, num_states),
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
		bitgrid_t bitgrid(b::size_each, b::_n_in.get_dim(), 0, 0, 0);

		grid_cell_t min = std::numeric_limits<grid_cell_t>::max(),
			max = std::numeric_limits<grid_cell_t>::min(); // any better alternative?

		for(const point& p2 : bitgrid.points())
		{
			const point offs = p2 - b::center;
			const grid_cell_t* const ptr = cell_ptr + (grid_cell_t)(offs.y * dim.width() + offs.x);
			bitgrid[p2] = *ptr;
			min = std::min(min, *ptr);
			max = std::max(max, *ptr);
		}

		// todo: better hashing function for not exactly n bits?
		const bool in_range = (min >= 0 && max < (int)b::own_num_states);
		return in_range
			? table[bitgrid.raw_value()]
			: *cell_ptr; // can not happen, except for border -> don't change
	}
private:
	//! version for multi-targets
	template<class T, class CT>
	int tar_write(const uint64_t& val, typename CT::cell_t *cell_tar,
		const _dimension<T>& tar_dim) const
	{
		const bitgrid_t tar_grid(b::size_each, b::_n_in.get_dim(), 0, val);
		for(const point& p : _n_out)
		{
			const auto ptr = cell_tar + (p.y * tar_dim.width()) + p.x;
			*ptr = tar_grid[p];
		}
		return tar_grid[center_out];
	}
public:
	//! version for multi-targets
	template<class T, class GCT>
	int calculate_next_state(const typename GCT::cell_t *cell_ptr,
		const _point<T>& p, const _dimension<T>& dim, typename GCT::cell_t *cell_tar,
		const _dimension<T>& tar_dim) const
	{
		// TODO: class member?
		using grid_cell_t = typename GCT::cell_t;

		(void)p; // for a ca, the coordinates are no cell input
	//	bitgrid_t bitgrid(b::size_each, b::_n_in.get_dim(), 0, 0, 0);
		bitgrid_t bitgrid(b::size_each, dimension(b::_n_in.size(), 0), 0, 0, 0);

		grid_cell_t min = std::numeric_limits<grid_cell_t>::max(),
			max = std::numeric_limits<grid_cell_t>::min(); // any better alternative?

		for(const auto& _p : ca::counted(_n_in))
		{
			const point& p = _p;
			const grid_cell_t* const ptr = cell_ptr + (grid_cell_t)(p.y * dim.width() + p.x);
			bitgrid[point(_p.id(), 0)] = *ptr;
			// TODO: if min is reset, it can maybe note be max? -> faster....
			min = std::min(min, *ptr);
			max = std::max(max, *ptr);
		}

		// todo: better hashing function for not exactly n bits?
		const bool in_range = (min >= 0 && max < (int)b::own_num_states);

		return in_range
			? tar_write<T, GCT>(table.at(bitgrid.raw_value()), cell_tar, tar_dim)
			: *cell_ptr; // can not happen, except for border -> don't change
	}

};

/*
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
};*/

using table_t = _table_t<std::vector, def_coord_traits, def_cell_traits>;

}}

#endif // CA_TABLE_H
