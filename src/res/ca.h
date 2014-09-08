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

#ifndef CA_H
#define CA_H

#include <map>
#include <stack>
#if 0
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#endif

#include "ca_basics.h"
#include "equation_solver.h"

namespace sca { namespace ca {

// TODO:
#define TABLE_OPTIMIZATION

//#define CA_DEBUG

class ca_eqsolver_t
{
	eqsolver::expression_ast ast;
	int* helper_vars = nullptr; //!< @todo: auto_ptr
	int helpers_size;
	int _border_width;
	n_t _n_in, _n_out;
	point center_cell;
protected:
	int num_states;
//	n_t_const neighbourhood;

protected:
	~ca_eqsolver_t() noexcept { delete[] helper_vars; }

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	ca_eqsolver_t(const char* equation, unsigned num_states = 0);

	template<class T, class CT>
	int calculate_next_state(const typename CT::cell_t *cell_ptr,
		const _point<T>& p, const _dimension<T>& dim) const
	{
		// TODO: replace &((*old_grid)[internal]) by old_value
		// and make old_value a ptr/ref?
		// TODO: const cast
		eqsolver::const_grid_storage_array arr(cell_ptr, dim.width());
		int result;
		eqsolver::grid_storage_single tar(&result);

		// TODO: why do we need to specify the default argument?
		using vprinter_t = eqsolver::_variable_print<>;
		vprinter_t vprinter(
			p.x, p.y,
			arr, tar, helper_vars);
		eqsolver::ast_print<vprinter_t> solver(&vprinter);
		return (int)solver(ast);
	}

	//! version for multi-targets
	template<class T, class CT>
	int calculate_next_state(const typename CT::cell_t *cell_ptr,
		const _point<T>& p, const _dimension<T>& dim, const typename CT::cell_t *cell_tar,
		const _dimension<T>& tar_dim) const
	{
		// TODO: replace &((*old_grid)[internal]) by old_value
		// and make old_value a ptr/ref?
		// TODO: const cast
		eqsolver::const_grid_storage_array arr(cell_ptr, dim.width());
		eqsolver::grid_storage_array tar(cell_tar, tar_dim.width());

		// TODO: why do we need to specify the default argument?
		using vprinter_t = eqsolver::_variable_print<>;
		vprinter_t vprinter(
			p.x, p.y,
			arr, tar, helper_vars);
		eqsolver::ast_print<vprinter_t> solver(&vprinter);
		return (int)solver(ast);
	}


	//! Runtime: depends on formula.
	// TODO: bit storage grids?
	template<class T, class = void>
	int calculate_next_state(uint64_t grid_int, uint64_t size_each,
		const _point<T>& p, const _dimension<T>& dim) const
	{
		int eval_idx = dim.width() * p.y + p.x; // TODO: bw?
		eqsolver::grid_storage_bits arr(grid_int, size_each, dim.width(), eval_idx);
		int _result;
		eqsolver::grid_storage_single tar(&_result);

		using vprinter_t = eqsolver::_variable_print<eqsolver::grid_storage_bits>;
		vprinter_t vprinter(
			p.x, p.y,
			arr, tar, helper_vars);
		eqsolver::ast_print<vprinter_t> solver(&vprinter);
		return solver(ast);
	}

public:
	int border_width() const noexcept { return _border_width; }
	using n_t_type = n_t;
	const n_t& n_in() const noexcept { return _n_in; }
	const n_t& n_out() const noexcept { return _n_out; }
	//bool can_optimize_table() const { return num_states }
};

using bitgrid_traits = coord_traits<char>;
using bitgrid_cell_traits = cell_traits<char>;

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
	bitgrid_t(storage_t each, const dimension& dim, u_coord_t border_width, cell_t fill = 0, cell_t border_fill = 0) :
		grid_alignment_t(dim, border_width),
		each(each),
		bitmask((1<<each)-1),
		grid(0) // start at zero and then or
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

	bit_reference operator[](point p) noexcept
	{
		//return (grid >> (index(p)*each)) & bitmask;
		return bit_reference(grid, index_h(p)*each, bitmask);
	}

	const const_bit_reference operator[](point p) const noexcept
	{
		return const_bit_reference(grid, index_h(p)*each, bitmask);
		//return (grid >> (index(p)*each)) & bitmask;
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const bitgrid_t& g) {
		const bit_storage_w str(g.grid, g.each);
		const number_grid ng{};
		const ::dimension tmp_dim(g._dim.width(), g._dim.height()); // TODO
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
};

//! header structure and basic utils for a table file
class _ca_table_hdr_t : public ca_eqsolver_t
{
protected:
	using base = ca_eqsolver_t;

	using u_coord_t = typename bitgrid_traits::u_coord_t;
	using cell_t = typename bitgrid_cell_traits::cell_t;

	using coord_t = typename bitgrid_traits::coord_t;
	using point = _point<bitgrid_traits>;
	using dimension = _dimension<bitgrid_traits>;
public:
	using n_t_type = _n_t<bitgrid_traits, std::vector<point>>;
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
	const unsigned own_num_states;
	const u_coord_t size_each; // TODO: u_coord_t
	const point center;

	// data for outside:
	const u_coord_t bw; // TODO: u_coord_t
	const n_t_type neighbourhood;

	u_coord_t compute_bw() const noexcept
	{
		return (n_w - 1)>>1;
	}

	n_t_type compute_neighbourhood() const noexcept
	{
		u_coord_t bw = border_width();
		u_coord_t n_width = (bw<<1) + 1;
		dimension moore = { n_width, n_width };
		return n_t_type(moore, point(bw, bw));
	}
public:
	unsigned border_width() const noexcept { return bw; } // TODO: should ret reference
	const n_t_type& n_in() const noexcept { return neighbourhood; }
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

	_ca_table_hdr_t(std::istream& stream);

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_ca_table_hdr_t(const char* equation, cell_t num_states = 0);
};

template<template<class ...> class TblCont>
class _ca_table_t : public _ca_table_hdr_t // TODO: only for reading?
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
		_ca_table_hdr_t::dump(stream);
		stream.write((char*)table.data(), table.size() * 8);
	}

	_ca_table_t(std::istream& stream) :
		_ca_table_hdr_t(stream),
		table(fetch_tbl(stream, size_each, n_w))
	{
	}

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_ca_table_t(const char* equation, cell_t num_states = 0) :
		_ca_table_hdr_t(equation, num_states),
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

using ca_table_t = _ca_table_t<std::vector>;

namespace calc_methods
{

/*class eq_t : private ca_eqsolver_t
{
	using ca_eqsolver_t::ca_eqsolver_t;
};

class big_table_t
{
	big_table_t(const char* equation)
	{
		ca_eqsolver_t solver(equation);
	}
};*/

}

#if 0
//! base class for all ca classes with virtual functions
class base
{
public:
	//! calculates next state at (human) position (x,y)
	//! if the ca is asynchronous, the function returns the next state
	//! as if the cell was active
	//! @param dim the grids *internal* dimension
	virtual int next_state(const int *cell_ptr, const point& p, const dimension& dim) const = 0;

	//! overload with human coordinates and reference to grid. slower.
	virtual int next_state(const grid_t &grid, const point& p) const = 0;

	//! returns whether cell at point @a p will change if active
	//! @a result is set to the result in all cases, if it is not nullptr
	virtual bool is_cell_active(const grid_t& grid, const point& p, cell_t* result = nullptr) const = 0;

	//! prepares the ca
	virtual void finalize() = 0;

	//! prepares the ca to run only on cells from @a sim_rect
	virtual void finalize(const rect& sim_rect) = 0;

	// TODO: function run_once_async()

	struct default_asynchronicity
	{
		bool operator()(unsigned ) const { return sca_random::get_int(2); }
	};

	struct synchronous
	{
		bool operator()(unsigned ) const { return true; }
	};

/*	//! runs the ca once, but only ever activating cells from @a sim_rect
	template<class Asynchronicity>
	virtual void run_once(const rect& sim_rect,
		const Asynchronicity& async = synchronous()) = 0;*/

	virtual void run_once(const rect& sim_rect) = 0;
	virtual void run_once(const rect& sim_rect, const default_asynchronicity& async = default_asynchronicity()) = 0;

	//! runs the whole ca
	//template<class Asynchronicity>
	virtual void run_once() = 0;
	virtual void run_once(const default_asynchronicity& async = default_asynchronicity()) = 0;

	virtual const std::vector<point>& active_cells() const = 0;
	virtual bool has_active_cells() const = 0;

	//! returns true iff not all cells are inactive
	virtual bool can_run() const = 0;

	//! returns the current grid
	virtual grid_t& grid() = 0;
	virtual const grid_t& grid() const = 0;

};
#endif

class input_ca
{
protected:
	using cell_t = typename def_cell_traits::cell_t;
	using u_coord_t = typename def_coord_traits::u_coord_t;
public:
	//! calculates next state at (human) position (x,y)
	//! if the ca is asynchronous, the function returns the next state
	//! as if the cell was active
	//! @param dim the grids *internal* dimension
	virtual int next_state(const int *cell_ptr, const point& p) const = 0;

	//! overload with human coordinates and reference to grid. slower.
	virtual int next_state(const point& p) const = 0;

	//! returns whether cell at point @a p will change if active
	//! @a result is set to the result in all cases, if it is not nullptr
	virtual bool is_cell_active(const point& p, cell_t* result = nullptr) const = 0;

	//! prepares the ca - *must* be called before the first run
	//! and *after* assigning the grid
	virtual void finalize() = 0;

	//! prepares the ca to run only on cells from @a sim_rect
	virtual void finalize(const rect& sim_rect) = 0;

	// TODO: function run_once_async()

	struct default_asynchronicity
	{
		bool operator()(unsigned ) const { return sca_random::get_int(2); }
	};

	struct synchronous
	{
		bool operator()(unsigned ) const { return true; }
	};

/*	//! runs the ca once, but only ever activating cells from @a sim_rect
	template<class Asynchronicity>
	virtual void run_once(const rect& sim_rect,
		const Asynchronicity& async = synchronous()) = 0;*/

	virtual void run_once(const rect& sim_rect) = 0;
	virtual void run_once(const rect& sim_rect, const default_asynchronicity& async) = 0;

	//! runs the whole ca
	//template<class Asynchronicity>
	virtual void run_once() = 0;
	virtual void run_once(const default_asynchronicity& async) = 0;

	virtual const std::vector<point>& active_cells() const = 0;
	virtual bool has_active_cells() const = 0;

	//! returns true iff not all cells are inactive
	virtual bool is_stable() const = 0;

	//! returns the current grid
	virtual grid_t& grid() = 0;
	virtual const grid_t& grid() const = 0;

	enum class stable_t
	{
		always,
		unknown,
		never
	};

	//! whether we know if the ca will get stable
	//! @todo: give asynch as param
	virtual stable_t gets_stable() const = 0;

	//! if gets_stable() returns always, this will get to an end
	//! configuration. if it returns never, this must return false.
	//! otherwise, the computation can take forever
	virtual bool stabilize() = 0;

	virtual void input(const point& p) = 0;

	virtual u_coord_t border_width() const = 0;

	virtual ~input_ca() {}
};

/**
 * @brief This class holds anything a cellular automaton's function
 * needs to know.
 *
 * Thus it contains no grid.
 */
template<class Solver>
class _ca_calculator_t : public Solver
{
	using _base = Solver;
/*	using cell_t = typename Traits::cell_t;
	using grid_t = _grid_t<Traits>;
	using point = _point<Traits>;
	using dimension = _dimension<Traits>;*/
	template<class CellTraits>
	using m_cell = typename CellTraits::cell_t;
public:
	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_ca_calculator_t(const char* equation, unsigned num_states = 0) :
		Solver(equation, num_states)
	{
	}

	//! tries to synch (TODO: better word) the CA from a file
	_ca_calculator_t(std::istream& stream) :
		Solver(stream)
	{
	}

	//! calculates next state at (human) position (x,y)
	//! @param dim the grids internal dimension
	template<class T, class CT>
	int next_state(const m_cell<CT> *cell_ptr, const _point<T>& p, const _dimension<T>& dim) const
	{
		return _base::template calculate_next_state<T, CT>(cell_ptr, p, dim);
	}

	//! calculates next states around (human) position (x,y)
	//! @param dim the grids internal dimension
	template<class T, class CT>
	int next_state(const m_cell<CT> *cell_ptr, const _point<T>& p, const _dimension<T>& dim,
		const m_cell<CT> *cell_tar, const _dimension<T>& tar_dim) const
	{
		return _base::template calculate_next_state<T, CT>(cell_ptr, p, dim, cell_tar, tar_dim);
	}

	//! overload, with x and y in internal format. slower.
	template<class T, class CT>
	int next_state_realxy(const m_cell<CT> *cell_ptr, const  _point<T>& p, const _dimension<T>& dim) const
	{
		int bw = _base::border_width();
		return next_state<T, CT>(cell_ptr, p - point { bw, bw }, dim);
	}

	//! overload with human coordinates and reference to grid. slower.
	template<class T, class CT>
	int next_state(const _grid_t<T, CT> &grid, const _point<T>& p) const
	{
		return next_state<T, CT>(&grid[p], p, grid.internal_dim());
	}

	//! returns whether cell at point @a p is active.
	//! @a result is set to the result in all cases, if it is not nullptr
	// TODO: overloads
	template<class T, class CT>
	bool is_cell_active(const _grid_t<T, CT>& grid, const _point<T>& p, m_cell<CT>* result = nullptr) const
	{
		const m_cell<CT>* const cell_ptr = &grid[p];
		const m_cell<CT> next = next_state<T, CT>(cell_ptr, p, grid.internal_dim());
		if(result)
		 *result = next;
		return next != *cell_ptr;
	}
};

using ca_calculator_t = _ca_calculator_t<ca_eqsolver_t>;

#if 0 // TODO: clang forbids this for some strange reason
class asm_synch_calculator_t
{
	static constexpr const std::array<point, 5> neighbour_points = {{{0,-1},{-1,0}, {0,0}, {1, 0}, {0,1}}};
	static constexpr const ca::n_t_constexpr<5> neighbours = ca::n_t_constexpr<5>(neighbour_points);
public:
	~asm_synch_calculator_t() { }

	asm_synch_calculator_t()
	{
	}

	static constexpr int get_border_width() { return 1; }

	//! calculates next state at (human) position (x,y)
	//! @param dim the grids internal dimension
	int next_state(const int *cell_ptr, const point& , const dimension& dim) const
	{
	/*	// TODO: replace &((*old_grid)[internal]) by old_value
		// and make old_value a ptr/ref?
		eqsolver::variable_print vprinter(dim.height(), dim.width(),
			p.x, p.y,
			cell_ptr, helper_vars);
		eqsolver::ast_print<eqsolver::variable_print> solver(&vprinter);
		return (int)solver(ast);*/
		// TODO: move to asm_basics?

		return (((*cell_ptr) & 3)
			+ (cell_ptr[1			]>>2)
			+ (cell_ptr[-1			]>>2)
			+ (cell_ptr[-dim.width()	]>>2)
			+ (cell_ptr[dim.width()	]>>2));

	//	return ((*cell_ptr) & 3) || ((cell_ptr[1]|cell_ptr[-1]|cell_ptr[-dim.width()]|cell_ptr[dim.width()])&3);
	}

	//! overload, with x and y in internal format. equally slow.
	int next_state_realxy(const int *cell_ptr, const point& p, const dimension& dim) const
	{
		return next_state(cell_ptr, p, dim);
	}

	//! overload with human coordinates and reference to grid. slower.
	int next_state(const grid_t &grid, const point& p) const
	{
		return next_state(&grid[p], p, grid.internal_dim());
	}

	//! returns whether cell at point @a p is active.
	//! @a result is set to the result in all cases, if it is not nullptr
	// TODO: overloads
	bool is_cell_active(const grid_t& grid, const point& p, cell_t* result = nullptr) const
	{
		const int* cell_ptr = &grid[p];
		/*cell_t next = next_state(cell_ptr, p, grid.dim());
		if(result)
		 *result = next;
		return next != *cell_ptr;*/
		// TODO: should a cell only be active if it changes its value?

		if(*cell_ptr & 4)
		{
			*result = next_state(cell_ptr, p, grid.internal_dim());
			return true;
		}
		return false;

		//return *cell_ptr & 4;
	}

	n_t_constexpr<5> get_neighbourhood() const
	{
		return neighbours;
	}

	// TODO: get states function
};
#endif

/**
 * Defines sequence: (stabilisation)((input)(stabilisation))*
 */
template<class Solver, class Traits, class CellTraits>
class ca_simulator_t : /*private _ca_calculator_t<Solver>,*/ public input_ca
{
	using point = _point<Traits>;
	using calc_class = _ca_calculator_t<Solver>;
	using grid_t = _grid_t<Traits, CellTraits>;

	using input_class = calc_class; //!< TODO: Solver class is enough

	calc_class ca_calc;
	input_class ca_input;

	grid_t _grid[2];
	grid_t *old_grid = _grid, *new_grid = _grid;
	typename calc_class::n_t_type n_in, n_out; // TODO: const?
	std::vector<point> //recent_active_cells(old_grid->size()),
			new_changed_cells; // TODO: this vector will shrink :/
	//! temporary variable
	std::set<point> cells_to_check; // TODO: use pointers here, like in grid
	int round = 0; //!< steps since last input
	bool async; // TODO: const?
	//using calc = _ca_calculator_t<Solver>;

	void initialize_first() { run_once(); }
public:
	ca_simulator_t(const char* equation, const char* input_equation,
		unsigned num_states, bool async = false) :
		ca_calc(equation, num_states),
		ca_input(input_equation, num_states),
		_grid{ca_calc.border_width(), ca_calc.border_width()},
		n_in(ca_calc.n_in()),
		n_out(ca_calc.n_out()),
		async(async)
	{
	}

	ca_simulator_t(const char* equation, const char* input_equation,
		bool async = false) : // TODO: 2 ctots?
		ca_simulator_t(equation, input_equation, 0, async)
	{
	}

	ca_simulator_t(const char* equation,
		bool async = false) :
		ca_simulator_t(equation, "v", async)
	{
	}

	virtual ~ca_simulator_t() {}

	// TODO: there is no virtual function right now...
	void reset_ca(const char* equation, const char* input_equation)
	{
		ca_calc = calc_class(equation);
		ca_input = input_class(input_equation);
		grid().resize_borders(ca_calc.border_width());
		n_in = ca_calc.n_in();
		n_out = ca_calc.n_out();
	}

	// TODO: no function should take grid pointer

	//! calculates next state at (human) position (x,y)
	//! @param dim the grids internal dimension
	int next_state(const int *cell_ptr, const point& p) const
	{
		(void) cell_ptr;
	//	return ca_calc.next_state(cell_ptr, p, grid().internal_dim());
		return (*new_grid)[p];
	}

	//! overload with human coordinates and reference to grid. slower.
	int next_state(const point& p) const
	{
	//	return ca_calc.next_state(grid(), p);
		return (*new_grid)[p];
	}

	//! returns whether cell at point @a p is active.
	//! @a result is set to the result in all cases, if it is not nullptr
	// TODO: overloads
	// TODO: faster than scanning the vector?
	bool is_cell_active(const point& p, typename grid_t::value_type* result = nullptr) const
	{
		//return ca_calc.is_cell_active(grid(), p, result);
		//return ca_calc.is_cell_active(old_grid, p, result);
		if(result)
		 *result =  (*new_grid)[p];
		return (*old_grid)[p] == (*new_grid)[p];
	}

	grid_t& grid() { return *old_grid; }
	const grid_t& grid() const { return *old_grid; }

	//! prepares the ca to run only on cells from @a sim_rect
	void finalize(const _rect<Traits>& sim_rect)
	{
		// incorrect if we finalize later? (what is 0 and 1?)
		_grid[1] = _grid[0]; // fit borders

		// make all cells active, but not those close to the border
		// TODO: make this generic for arbitrary neighbourhoods
		new_changed_cells.reserve(sim_rect.area());
		for( const point &p : sim_rect ) {
			// TODO: use active criterion if possible
			// TODO: otherwise, invariant can be broken...
			// TODO: (because these cells are not active)
			//for(const point np : n_in)
			if(ca_calc.is_cell_active(_grid[0], p))
			 new_changed_cells.push_back(p);
		}

		initialize_first();
	}

	//! prepares the ca - *must* be run before simulating
	void finalize() { finalize(_grid->human_dim()); }

	// TODO: function run_once_async()

	//! runs the ca once, but only ever activating cells from @a sim_rect
	template<class Asynchronicity>
	void _run_once(const rect& sim_rect,
		const Asynchronicity& async = synchronous())
	{
		old_grid = _grid + ((round+1)&1);
		new_grid = _grid + ((round)&1);

		cells_to_check.clear();
		for(const point& ap : new_changed_cells)
		for(const point& np : n_in)
		{
			const point p = ap + np;
			if(sim_rect.is_inside(p)) // TODO: intsct?
		//	if(!_grid->point_is_on_border(p))
			 cells_to_check.insert(p);
		}
		new_changed_cells.clear();

		// try to find neighbours that do not overwrite each other
		std::set<point> try_change;
		bool this_ok;
		do // TODO: this is slow...
		{
			this_ok = true;
			for(auto itr = cells_to_check.cbegin(); itr != cells_to_check.cend() && this_ok; ++itr)
			if(async(2))
			{
				const auto n_ok = [&](const point& p){ return try_change.find(p) == try_change.end(); };
				if(n_out.for_each_bool(*itr, n_ok))
				 try_change.insert(*itr);
				else
				 this_ok = false;
			}
		} while(!this_ok);

		*new_grid = *old_grid; // TODO: necessary?

	//	for(const point& p : cells_to_check )
	//	if(try_change.find(p) != try_change.end())
		for(const point& p : try_change )
		// TODO: use bool async template here to increase speed?
		// plus: exploit code duplication?
		{
			int new_value;
			const int old_value = (*old_grid)[p];
			(*new_grid)[p] = (new_value
				= ca_calc.template next_state<Traits, CellTraits>
					(&((*old_grid)[p]),
						p, _grid->internal_dim()));

		//	std::cout << "at " << p << ": " << new_value
		//		<< ", " << old_value << std::endl;
			if(new_value != old_value)
			{
				new_changed_cells.push_back(p);
			}

		}
/*		else
		{	// i.e. async + not activated
			// we still need to assign the old value:
			(*new_grid)[p] = (*old_grid)[p];
		}*/

	//	std::cout << "NOW:" <<  std::endl;
	//	std::cout << *old_grid;
	//	std::cout << *new_grid;

		++round;
	}

	// TODO: move up to virtual class
	//! active cells during a stabilisation
	// TODO: not all of these cells must be active
	const std::vector<point>& active_cells() const { return new_changed_cells; }
	bool has_active_cells() const { return !active_cells().empty(); }

	//! runs the whole ca
	template<class Asynchronicity>
	void _run_once(const Asynchronicity& async = synchronous())
	{
		run_once(_grid->human_dim(), async);
	}


	virtual void run_once(const rect& sim_rect) { _run_once(sim_rect, synchronous()); }
	virtual void run_once(const rect& sim_rect, const default_asynchronicity& async) {
		_run_once(sim_rect, async);
	}
	virtual void run_once(const rect& sim_rect, const synchronous& async) {
		_run_once(sim_rect, async);
	}


	//! runs the whole ca
	//template<class Asynchronicity>
	virtual void run_once() { _run_once(synchronous()); }
	virtual void run_once(const default_asynchronicity& async) {
		_run_once(async);
	}
	virtual void run_once(const synchronous& async) {
		_run_once(async);
	}


	//! returns true iff not all cells are inactive
	bool can_run() const {
		return (new_changed_cells.size() || async); // TODO: async condition is wrong
	}

	stable_t gets_stable() const { return stable_t::unknown; }

	bool is_stable() const { return false; } // TODO!!!!

	bool stabilize()
	{
		if(gets_stable() == stable_t::never)
		 return false;
		else
		{
			while(can_run()) { run_once(); }
			return true;
		}
	}

	// note: the input functions could be put into a different class
	// however, we currently don't think it would be practical

	void input(const point& p) // TODO: ret value?
	{
		//if(has_)
		// TODO: for now, we assume that the ca is always stable
		(*old_grid)[p] = (*new_grid)[p] = ca_input.template
			next_state<Traits, CellTraits>(*new_grid, p);
		/*for(const point np : n_in)
		{
			point cur = p + np;
			if(!grid().point_is_on_border(cur)
				&& ca_calc.is_cell_active(*new_grid, cur))
			 new_changed_cells.push_back(cur);
		}*/
		new_changed_cells.push_back(p);

		initialize_first();
	}

	typename Traits::u_coord_t border_width() const {
		return ca_calc.border_width();
	}
};

#if 0
class configuration_graph_types
{
#if 0
protected:
	struct vertex
	{
		configuration conf;
	};
	struct edge {};

	typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, vertex, edge> graph_t;
	typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
	typedef boost::graph_traits<graph_t>::edge_descriptor edge_t;

	typedef std::map<configuration, vertex_t> map_t;
	typedef std::stack<vertex_t> stack_t;
#endif
};


class scientific_ca_t : public ca_simulator_t, public configuration_graph_types
{
#if 0
	class asynchronicity
	{
		std::vector<bool> value;
	public:
		asynchronicity(std::size_t size) :
			value(size) {}

		void increase() {
			for(const bool& b : value) { (void)b; }
		}
		bool operator()(unsigned i) { return value[i]; }
	};

	// TODO: move down to public function?
	graph_t try_all(const rect& sim_rect) const
	{
		graph_t graph;
		stack_t stack;
		map_t map;

		// start vertex
		vertex_t v = boost::add_vertex(graph);
		graph[v].conf = configuration(std::set<point>(), grid());
		try_children(boost::add_vertex(graph), graph, map, stack, sim_rect);

		return graph;
	}

	void try_children(vertex_t node, graph_t& graph, map_t& map, stack_t& stack, const rect& sim_rect) const
	{
		map_t::const_iterator itr;
		configuration c(sim_rect, grid());
		itr = map.find(c);
		if(itr == map.end())
		{ // vertex has not been initialized yet
			vertex_t v = boost::add_vertex(graph);
		//	(*graph)[v].conf =
			map.insert(std::pair<configuration,
				vertex_t>(c, v));

			// Create an edge conecting those two vertices
			edge_t e; bool b;
			if(!stack.empty()) // root can not have parents
			 boost::tie(e,b) = boost::add_edge(stack.top(), v, graph);

			// get child vector
			std::vector<configuration> next_confs;

			// recurse
			stack.push(v);
			try_children(v, graph, map, stack, sim_rect);
			stack.pop();

			//std::cout << "Workgraph: erasing node " << graph[v].conf << std::endl;
		}
		else
		{
			std::cout << "Adding edge, but not node..." << std::endl;
			std::cout << "  (to: " << (graph[node].conf) << ")" << std::endl;

			boost::add_edge(itr->second, node, graph); // TODO: correct?
		}
	}

public:
	using ca_simulator_t::ca_simulator_t;

	graph_t get_all_configurations(const rect& sim_rect)
	{
		return try_all(sim_rect);
	}
#endif
};
#endif

} }

#endif // CA_H
