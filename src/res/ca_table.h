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

#include <bitset> // TODO

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
class _table_hdr_t// : public eqsolver_t // TODO: make base a template?
{
protected:
	using u_coord_t = typename bitgrid_traits::u_coord_t;
	using cell_t = typename bitgrid_cell_traits::cell_t;

	using coord_t = typename bitgrid_traits::coord_t;
	using point = _point<bitgrid_traits>;
	using dimension = _dimension<bitgrid_traits>;
	using rect = _rect<bitgrid_traits>;
	using grid_t = _grid_t<bitgrid_traits, bitgrid_cell_traits>;
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

	static int fetch_8(std::istream& stream);

	n_t fetch_n(std::istream& stream) const;
	void put_n(std::ostream &stream, const n_t &n) const;

public:
/*	unsigned border_width() const noexcept { return bw; } // TODO: should ret reference
	const n_t& n_in() const noexcept {
		std::set<n_t::>
	}
	const n_t& n_out() const noexcept {  }*/
protected:
	static int fetch_32(std::istream& stream);

	//! O(1)
	void dump(std::ostream& stream) const;

	void dump_as_grids(std::ostream& stream) const
	{
		const rect rect_max = rect_cover(
			_n_in.get_rect(), _n_out.get_rect());

		const ::point both_center = convert<def_coord_traits>(-rect_max.ul());

		::grid_t tmp_grid(convert<def_coord_traits>(dimension(rect_max.dx(), rect_max.dy())), 0);

		tmp_grid[both_center] = 1;
		stream << tmp_grid << std::endl;
		tmp_grid[both_center] = 0;

		convert<def_coord_traits>(_n_in).for_each(convert<def_coord_traits>(both_center), [&](const ::point& p){ tmp_grid[convert<def_coord_traits>(p)] = 1; });
		stream << tmp_grid << std::endl;
		convert<def_coord_traits>(_n_in).for_each(convert<def_coord_traits>(both_center), [&](const ::point& p){ tmp_grid[convert<def_coord_traits>(p)] = 0; });

		// it's getting boring...
		// TODO: callback for that
		convert<def_coord_traits>(_n_out).for_each(convert<def_coord_traits>(both_center), [&](const ::point& p){ tmp_grid[convert<def_coord_traits>(p)] = 1; });
		stream << tmp_grid << std::endl;
		convert<def_coord_traits>(_n_out).for_each(convert<def_coord_traits>(both_center), [&](const ::point& p){ tmp_grid[convert<def_coord_traits>(p)] = 0; });
	}

	_table_hdr_t(std::istream& stream);

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_table_hdr_t(_table_hdr_t::cell_t num_states, const n_t& _n_in, const n_t& _n_out);

	template<class Traits>
	typename Traits::u_coord_t calc_border_width() const noexcept { return bw; }
	template<class Traits>
	_n_t_v<Traits> calc_n_in() const {
		using point = _point<Traits>;
		std::vector<point> res_v;
		for(const auto& p : _n_in)
		 res_v.push_back(point(p.x, p.y));
		return _n_t_v<Traits>(std::move(res_v));
	}
	template<class Traits>
	_n_t_v<Traits> calc_n_out() const {
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


//	using b = _table_hdr_t; // TODO: remove b everywhere
//	using typename b::cell_t;
//	using typename b::point;

	static TblCont<uint64_t> fetch_tbl(std::istream& stream, unsigned size_each, unsigned n_size)
	{
		TblCont<uint64_t> res(1 << (size_each * n_size));
		stream.read((char*)res.data(), res.size() * 8);
		return res;
	}


	class from_equation
	{
		eqsolver_t eqs;
		const u_coord_t size_each;
		const n_t _n_out;
		const point center, center_out;
	public:
		from_equation(eqsolver_t&& eqs, u_coord_t size_each, const n_t& _n_out, const point& center) :
			eqs(eqs),
			size_each(size_each),
			_n_out(_n_out),
			center(center),
			center_out(_n_out.get_center_cell())
		{}

		u_int64_t operator()(const bitgrid_t& grid) const
		{
			// TODO!! thread safety!
			static dimension n_out_dim = _n_out.get_dim();
			static ::grid_t tmp_result(::dimension(n_out_dim.dx(), n_out_dim.dy()), 0);

			// TODO: tmp_result should be bitgrid.
			bitgrid_t bit_tmp_result(size_each, n_out_dim, 0, 0);
			eqs.calculate_next_state_grids(grid.raw_value(), size_each,
				/*::point(center.x, center.y)*/ center,
				grid.internal_dim(),
				&tmp_result[::point(center_out.x, center_out.y)],
				::dimension(n_out_dim.dx(), n_out_dim.dy()));
			for(const point& p : _n_out)
			 bit_tmp_result[center_out + p] = tmp_result[::point(center_out.x + p.x, center_out.y + p.y)];
			return bit_tmp_result.raw_value();
		}

	};

	class from_trans
	{
		const std::vector<trans_t>& tf;
		mutable std::vector<trans_t>::const_iterator itr;
		const u_coord_t size_each;
		const n_t _n_in, _n_out;
		const point center, center_out;

		template<class Grid, class Conf>
		bool grid_has_conf(const Grid& g, const Conf& c) const
		{
			conf_t::const_iterator citr = c.cbegin();
			const auto cb = [&](const _point<bitgrid_traits>& p) {
				return ((int)g[p]) == *(citr++);
			};
			return _n_in.for_each_bool(center, cb);
		}
	public:
		from_trans(const std::vector<trans_t>& tf, u_coord_t size_each, const n_t& _n_in, const n_t& _n_out, const point& center) :
			tf(tf),
			itr(tf.cbegin()),
			size_each(size_each),
			_n_in(_n_in),
			_n_out(_n_out),
			center(center),
			center_out(_n_out.get_center_cell())
		{}

		u_int64_t operator()(const bitgrid_t& grid) const
		{
			// TODO!! thread safety!
			//static dimension n_out_dim = _n_out.get_dim();

			bitgrid_t bit_tmp_result(size_each, dimension(_n_out.size(), 1), 0, 0);

			if(itr != tf.cend() &&
				grid_has_conf(grid, itr->get_full_input()))
			{
				std::cerr << "equal:" << grid << ", " << *itr << std::endl;

				for(std::size_t i = 0; i < _n_out.size(); ++i)
				 bit_tmp_result[point(i, 0)] = itr->get_output()[i];

				for(std::size_t i = 0; i < _n_out.size(); ++i)
				 bit_tmp_result[point(i, 0)] = itr->get_output()[i];
			//	 bit_tmp_result[center_out + p] = itr->get_output()[p.id()];

				++itr;
			}
			else
			{
				if(itr == tf.cend())
				 std::cerr << "differ:" << grid << ", " << "(end)" << std::endl;
				else
				 std::cerr << "differ:" << grid << ", " << *itr << std::endl;

				for(const auto& p : ca::counted(_n_out))
				 bit_tmp_result[point(p.id(), 0)] = grid[center + p];
			//	 bit_tmp_result[center_out + p] = grid[center + p];
			}

			std::cerr << grid << "->" << bit_tmp_result << std::endl;

			return bit_tmp_result.raw_value();
		}

	};

	// TODO : static?
	//! used to dump an in-memory-table from an equation
	template<class Functor>
	TblCont<uint64_t> calculate_table(const Functor& ftor) const
	{
		TblCont<uint64_t> tbl;
		tbl.resize(1 << (size_each * _n_in.size())); // ctor can not reserve

		const dimension n_in_dim(_n_in.get_dim().dx(), _n_in.get_dim().dy());
		bitgrid_t grid(size_each, n_in_dim, 0, 0);
		const std::size_t max = (int)pow(own_num_states, _n_in.size());

		std::size_t percent = 0, cur;
		std::cerr << "Precalculating table, please wait..." << std::endl;

		bitgrid_t tbl_idx(size_each, dimension(_n_in.size(), 1), 0, 0);

		for(std::size_t i = 0; i < max; ++i)
		{
			// evaluate
//			std::cerr << "tbl idx: " << (int)tbl_idx.raw_value() << std::endl;

			tbl.at(tbl_idx.raw_value()) = ftor(grid);

//			std::cerr << grid << " -> " << tmp_result << std::endl;

#ifdef SCA_DEBUG
//			if(tbl.at(grid.raw_value()) != grid[center])
//			 std::cerr << grid << " => " << tbl.at(grid.raw_value()) << std::endl;
#endif

			// stats
			cur = (i * 100) / max; // max can never be 0 // TODO: 100 / max = const
			if(percent < cur)
			{
				percent = cur;
				std::cerr << "..." << percent << " percent" << std::endl;
			}

			// increase
			{
				bool go_on = true;

				int digit = _n_in.size() - 1; // count up the counter backwards
				for(auto itr = _n_in.neighbours().crbegin();
					go_on && (itr != _n_in.neighbours().crend()); ++itr)
				{
					const point& p = *itr;
		//			std::cerr << "increasing: " << p << " = " << (center + p) << std::endl;

					bit_reference r = grid[center + p];
					go_on = ((r = ((r + 1) % own_num_states)) == 0);

					bit_reference r2 = tbl_idx[point(digit, 0)];
					r2 = ((r2 + 1) % own_num_states);

					--digit;
				}
			}
		}

		return tbl;
	}


	//! used to dump an in-memory-table from an equation
	TblCont<uint64_t> calculate_table_eq(eqsolver_t&& eqs) const
	{
		return calculate_table(
			from_equation(std::move(eqs),
				size_each, _n_out, center));
	}

	//! used to dump an in-memory-table from a vector of transitions
	TblCont<uint64_t> calculate_table_trans(const std::vector<trans_t>& tf) const
	{
		return calculate_table(
			from_trans(tf,
				size_each, _n_in, _n_out, center));
	}

public:
	//! O(table)
	void dump(std::ostream& stream) const
	{
		_table_hdr_t::dump(stream);
		stream.write((char*)table.data(), table.size() * 8);
	}


	void dump_as_grids(std::ostream& stream) const
	{
		// TODO: find out symmetry...
		bool _rot = false, _mirr = false;

		stream << _rot << std::endl << _mirr << " "/*<< std::endl*/;

		_table_hdr_t::dump_as_grids(stream);

		::grid_t grid_in(convert<def_coord_traits>(_n_in.get_dim()), 0),
			grid_out(convert<def_coord_traits>(_n_out.get_dim()), 0);

		for(uint64_t j = 0; j < table.size(); ++j)
		{
			grid_in.reset(0);
			grid_out.reset(0);

			bitgrid_t tbl_idx(size_each, dimension(_n_in.size(), 1), 0, j);

			std::cout << "j:" << j << ", tbl_idx: "<< tbl_idx<< std::endl;

			for(std::size_t i = 0; i < _n_in.size(); ++i)
			 grid_in[convert<def_coord_traits>(center + _n_in[i])] = tbl_idx[point(i, 0)];

			for(std::size_t i = 0; i < _n_out.size(); ++i)
			 grid_out[convert<def_coord_traits>(center_out + _n_out[i])] = table[j];

			if(grid_in != grid_out) // TODO: n size...
			 stream << grid_in << std::endl << grid_out << std::endl;
		}
	}

	_table_t(std::istream& stream) :
		_table_hdr_t(stream),
		table(fetch_tbl(stream, size_each, _n_in.size()))
	{
	}

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_table_t(eqsolver_t _eqs, cell_t num_states = 0) :
		_table_hdr_t(num_states,
			_eqs.calc_n_in<bitgrid_traits>(),
			_eqs.calc_n_out<bitgrid_traits>()),
		table(calculate_table_eq(std::move(_eqs)))
	{
	}

	_table_t(const std::vector<trans_t>& tf,
		cell_t num_states,
		const n_t& n_in,
		const n_t& n_out) :
		_table_hdr_t(num_states, n_in, n_out),
		table(calculate_table_trans(tf))
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
		bitgrid_t bitgrid(size_each, _n_in.get_dim(), 0, 0, 0);

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

private:
	//! version for multi-targets
	template<class T, class CT>
	int tar_write(const uint64_t& val, typename CT::cell_t *cell_tar,
		const _dimension<T>& tar_dim) const
	{
		const bitgrid_t tar_grid(size_each, dimension(_n_out.size(), 1), 0, val);
		// copy tar_grid's result to *ptr
		for(const auto& _p : ca::counted(_n_out))
		{
			const point& p = _p;
			const auto ptr = cell_tar + (p.y * (coord_t)tar_dim.width()) + p.x;
			if(*ptr != tar_grid[point(_p.id(), 0)]) {
				std::cout << "ptr:" << *ptr << std::endl;
			std::cout << "p: " << p << ", tar_grid: " << tar_grid << " => " << tar_grid[point(_p.id(), 0)] << std::endl;
			}
			*ptr = tar_grid[point(_p.id(), 0)];
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
	//	bitgrid_t bitgrid(size_each, _n_in.get_dim(), 0, 0, 0);
		bitgrid_t bitgrid(size_each, dimension(_n_in.size(), 1), 0, 0, 0);

		grid_cell_t min = std::numeric_limits<grid_cell_t>::max(),
			max = std::numeric_limits<grid_cell_t>::min(); // any better alternative?

		for(const auto& _p : ca::counted(_n_in))
		{
			const point& p2 = _p;
		/*	std::cout << typeid(decltype(_p)).name() << std::endl;
			std::cout << p2.y << "," << (coord_t)dim.width()<<  ", " << p2.x << std::endl;
			std::cout << (coord_t)(p2.y * (coord_t)dim.width() + p2.x) << std::endl;*/
			const grid_cell_t* const ptr = cell_ptr + (coord_t)(p2.y * (coord_t)dim.width() + p2.x); // TODO: check signs...
			bitgrid[point(_p.id(), 0)] = *ptr;
			// TODO: if min is reset, it can maybe not be max? -> faster....
//			std::cout << *ptr << std::endl;
			min = std::min(min, *ptr);
			max = std::max(max, *ptr);

	//					std::cout << "NN " << (int)(coord_t)(p2.y * dim.width() + p2.x) << std::endl;

		}

		// todo: better hashing function for not exactly n bits?
		const bool in_range = (min >= 0 && max < (int)own_num_states);
//std::cout << p << " -> raw (falschrum): " << bitgrid << std::endl;

	#if 0
		bitgrid_t tmp(size_each, dimension(_n_out.size(), 1), 0, table.at(bitgrid.raw_value()));

		if(in_range) {
//		bitgrid_t tmp(size_each, dimension(_n_out.size(), 1), 0, table.at(bitgrid.raw_value()));
		if(tmp!=bitgrid)
		{
		 std::cout << p << " -> raw (falschrum): " << bitgrid << std::endl;
		std::cout << " -> table:" << tmp << std::endl;
		for(const auto& p2 : _n_out)
		{
			const grid_cell_t* const ptr1 = cell_ptr + (coord_t)(p2.y * (coord_t)dim.width() + p2.x);
			const grid_cell_t* const ptr2 = cell_tar + (coord_t)(p2.y * (coord_t)tar_dim.width() + p2.x);
			if(*ptr1 != *ptr2)
			{
				std::cout << "p, p2: " << p << ", " << p2 << " -> " << *ptr1 << " != " << *ptr2 << std::endl;
				throw "up";
			}
		}
		}
		}

		std::cout << p << ": " << bitgrid << std::endl;
		return (in_range && (tmp!=bitgrid))
			? tar_write<T, GCT>(table.at(bitgrid.raw_value()), cell_tar, tar_dim)
			: *cell_ptr; // can not happen, except for border -> don't change
#else
		if(in_range)
		{
			bitgrid_t tmp(size_each, dimension(_n_out.size(), 1), 0, table.at(bitgrid.raw_value()));

			if(tmp!=bitgrid)
			{
			 std::cout << p << " -> raw (falschrum): " << bitgrid << std::endl;
			std::cout << " -> table:" << tmp << std::endl;
			for(const auto& p2 : _n_out)
			{
				const grid_cell_t* const ptr1 = cell_ptr + (coord_t)(p2.y * (coord_t)dim.width() + p2.x);
				const grid_cell_t* const ptr2 = cell_tar + (coord_t)(p2.y * (coord_t)tar_dim.width() + p2.x);
				if(*ptr1 != *ptr2)
				{
					std::cout << "p, p2: " << p << ", " << p2 << " -> " << *ptr1 << " != " << *ptr2 << std::endl;
				//	throw "up";
				}
			}
			}

			return (tmp!=bitgrid)
			? tar_write<T, GCT>(table.at(bitgrid.raw_value()), cell_tar, tar_dim)
			: *cell_ptr;

		}
		else
		 return *cell_ptr;

#endif
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
