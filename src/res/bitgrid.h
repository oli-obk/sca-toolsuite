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

#ifndef BITGRID_H
#define BITGRID_H

#include "grid.h" // TODO: actually grid_alignment.h

class bit_reference_base
{
protected:
	uint64_t minbit, bitmask;
	bit_reference_base(uint64_t minbit, uint64_t bitmask) noexcept:
		minbit(minbit),
		bitmask(bitmask) {}
	uint64_t bits_in(const uint64_t& grid) const noexcept {
		return (grid >> minbit) & bitmask;
	}
};

class const_bit_reference : public bit_reference_base
{
	const uint64_t& grid;
public:
	const_bit_reference(const uint64_t& grid, uint64_t minbit, uint64_t bitmask) noexcept:
		bit_reference_base(minbit, bitmask),
		grid(grid)
	{
	}

public:
	operator uint64_t () const noexcept { return bits_in(grid); }
};

class bit_reference : public bit_reference_base
{
// TODO:
//	friend class bitgrid_t; // TODO?
//	reference();	//! no public constructor
	//u_coord_t minbit;

	uint64_t& grid;
public:
	bit_reference(uint64_t& grid, uint64_t minbit, uint64_t bitmask) noexcept:
		bit_reference_base(minbit, bitmask),
		grid(grid)
	{
	}
public:
	operator uint64_t () const noexcept { return bits_in(grid); }

	//! TODO: faster ops for ++ etc.
	bit_reference& operator= (const uint64_t c) noexcept {
		grid = grid & (~(bitmask << minbit));
		grid = grid | (c << minbit);
		return *this;
	}
	bit_reference& operator++() noexcept {
		grid += (uint64_t)(1 << minbit);
		return *this;
	}
};

using bitgrid_traits = coord_traits<int64_t>;
using bitgrid_cell_traits = cell_traits<int64_t>;

class bitcell_itr_base
{
protected:
	using coord_t = typename bitgrid_traits::coord_t;
	using u_coord_t = typename bitgrid_traits::u_coord_t;
	using dimension = _dimension<bitgrid_traits>;
	using storage_t = uint64_t;
	const storage_t each;
	coord_t linewidth;
	//cell_t *ptr, *next_line_end;
	u_coord_t ptr, next_line_end;
	coord_t bw_2;
	const storage_t bitmask;

public:
	bitcell_itr_base(storage_t each, dimension dim, coord_t bw,
		bool pos_is_begin = true) noexcept:
		each(each),
		linewidth(dim.width()),
		ptr(0 +
			((pos_is_begin) ? bw * (linewidth+1)
			: dim.area() - bw * (linewidth-1)) ),
		next_line_end(ptr + linewidth - (bw << 1)),
		bw_2(bw << 1),
		bitmask((1 << each) - 1)
	{
	}

	bitcell_itr_base& operator++() noexcept
		// TODO: the return value of this is a bug
	{
		// TODO: use a good modulo function here -> no if
		if((/*grid = grid >> each,*/ ++ptr) == next_line_end)
		{
			ptr += bw_2; // TODO: ptr is somehow unneeded
			next_line_end += linewidth;
		//	grid = grid >> (bw_2 * each);
		}
		// std::cout << "pos now: " << ptr << std::endl;
		return *this;
	}

	bool operator==(const bitcell_itr_base& rhs) const noexcept {
		return ptr == rhs.ptr; }
	bool operator!=(const bitcell_itr_base& rhs) const noexcept {
		return !operator==(rhs); }
};

class const_bitcell_itr : public bitcell_itr_base
{
	storage_t grid;
public:
	const_bitcell_itr(storage_t grid, storage_t each, dimension dim, coord_t bw,
		bool pos_is_begin = true) noexcept:
		bitcell_itr_base(each, dim, bw, pos_is_begin),
		grid(grid) {}

	const_bit_reference operator*() const noexcept {
		return const_bit_reference(grid, ptr * each, bitmask);
	}
};

class bitcell_itr : public bitcell_itr_base
{
	storage_t& grid;
public:
	bitcell_itr(storage_t& grid, storage_t each, dimension dim, coord_t bw,
		bool pos_is_begin = true) noexcept:
		bitcell_itr_base(each, dim, bw, pos_is_begin),
		grid(grid) {}

	bit_reference operator*() noexcept {
		return bit_reference(grid, ptr * each, bitmask);
	}
};

class bitgrid_t : public grid_alignment_t<bitgrid_traits>
{
	using cell_t = typename bitgrid_cell_traits::cell_t;
	using point = _point<bitgrid_traits>;

	using storage_t = uint64_t;
	const storage_t each, bitmask;
	storage_t grid;

	void size_check()
	{
		if(_dim.area() * each > 64)
		{
			std::cout << _dim << std::endl;
			std::cout << each << std::endl;
			throw "Error: grid too large for 64 bit integer.";
		}
	}
public:
	template<class Traits>
	bitgrid_t(storage_t each, const _dimension<Traits>& dim,
		u_coord_t border_width, cell_t fill, cell_t border_fill) :
		grid_alignment_t(dim, border_width),
		each(each),
		bitmask((1<<each)-1),
		grid(0) // start at zero and then or
		// TODO: first redirect to raw ctor?
	{
		size_check();
		// we do not know a more simple collective operation than this
		// TODO: only fill border
		for(unsigned char pos = 0; pos < storage_area(); ++pos)
		{
			grid = grid | ((uint64_t)(border_fill) << (each * pos));
		}

		// 2nd for loop: cache does not matter
		//for(const bit_reference& b : this)
		for(auto b = this->begin(); b != this->end(); ++b)
		{
			*b = (uint64_t)fill;
		}

	/*	(void)fill; // TODO
		(void)border_fill;*/
	}

	//! constructs bitgrid from raw value. useful for serialization
	template<class Traits>
	bitgrid_t(storage_t each, const _dimension<Traits>& dim,
		u_coord_t border_width, storage_t raw) :
		grid_alignment_t(dim, border_width),
		each(each),
		bitmask((1<<each)-1),
		grid(raw)
	{
	}

	template<class Traits>
	bit_reference operator[](_point<Traits> p) noexcept
	{
		//return (grid >> (index(p)*each)) & bitmask;
		return bit_reference(grid, index_h(p)*each, bitmask);
	}

	template<class Traits>
	const const_bit_reference operator[](_point<Traits> p) const noexcept
	{
		return const_bit_reference(grid, index_h(p)*each, bitmask);
		//return (grid >> (index(p)*each)) & bitmask;
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const bitgrid_t& g) {
		const bit_storage_w str(g.grid, g.each);
		const number_grid ng{};
		const ::dimension tmp_dim(g._dim.dx(), g._dim.dy()); // TODO
		write_grid(&ng, stream, tmp_dim, g.bw, str);
		return stream;
	}

	//TODO: reference class like in std::vector<bool>?
//	cell_t get(point p) { return (grid >> (index(p)*each)) & bitmask; }
//	void set(point p, const cell_t& value) { return grid = grid & (bitmask << (index(p)*each)); }

	using iterator = bitcell_itr;
	using const_iterator = const_bitcell_itr;

	iterator begin() noexcept { return iterator(grid, each, _dim, bw); }
	iterator end() noexcept { return iterator(grid, each, _dim, bw, false); }
	const_iterator cbegin() const noexcept { return const_iterator(grid, each, _dim, bw); }
	const_iterator cend() const noexcept { return const_iterator(grid, each, _dim, bw, false); }

	storage_t raw_value() const noexcept { return grid; }

	bool operator==(const bitgrid_t& other) const noexcept {
		return grid == other.grid;
	}
	bool operator!=(const bitgrid_t& other) const noexcept {
		return ! operator==(other);
	}

};


#endif // BITGRID_H
