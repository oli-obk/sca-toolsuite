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
protected:
	int num_states;
//	n_t_const neighbourhood;

protected:
	~ca_eqsolver_t() { delete[] helper_vars; }

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	ca_eqsolver_t(const char* equation, unsigned num_states = 0)
		: num_states(num_states)
	{
	//	debug("Building AST from equation...\n");
		eqsolver::build_tree(equation, &ast);

		eqsolver::ast_area<eqsolver::variable_area_grid>
			grid_solver;
		_border_width = (int)grid_solver(ast);
#ifdef CA_DEBUG
		printf("Size of Moore Neighbourhood: %d\n", // TODO: use cout
			_border_width);
#endif
		eqsolver::ast_area<eqsolver::variable_area_helpers>
			helpers_solver;
		helpers_size = (int)helpers_solver(ast) + 1;
#ifdef CA_DEBUG
		printf("Size of Helper Variable Array: %d\n",
			helpers_size);
#endif
		if(helpers_size > 0)
		 helper_vars = new int[helpers_size];

#if 0
		eqsolver::ast_minmax minmax_solver(helpers_size);
		//std::pair<int, int> mm = (std::pair<int, int>)minmax_solver(ast);
	/*	num_states = (mm.first == INT_MIN || mm.second == INT_MAX)
			? INT_MAX
			: (mm.second - mm.first + 1);*/
		std::pair<eqsolver::expression_ast, eqsolver::expression_ast> mm
			= (std::pair<eqsolver::expression_ast, eqsolver::expression_ast>)minmax_solver(ast);

		eqsolver::ast_dump dumper;
		std::cout << "original: " << dumper(ast) << std::endl;

		std::cout << "mm first: " << (std::string)dumper(mm.first) << std::endl; // TODO: non return syntax
		(void)mm;
#endif
		//num_states = 0;
	}

	int calculate_next_state(const int *cell_ptr,
		const point& p, const dimension& dim) const
	{
		// TODO: replace &((*old_grid)[internal]) by old_value
		// and make old_value a ptr/ref?
		eqsolver::grid_storage_array arr(cell_ptr, dim.width());
		// TODO: why do we need to specify the default argument?
		using vprinter_t = eqsolver::variable_print<eqsolver::grid_storage_array>;
		vprinter_t vprinter(dim.height(), dim.width(),
			p.x, p.y,
			arr, helper_vars);
		eqsolver::ast_print<vprinter_t> solver(&vprinter);
		return (int)solver(ast);
	}

	int calculate_next_state(uint64_t grid_int, uint64_t size_each,
		const point& p, const dimension& dim) const
	{
		int eval_idx = dim.width() * p.y + p.x; // TODO: bw?
		eqsolver::grid_storage_bits arr(grid_int, size_each, dim.width(), eval_idx);

		using vprinter_t = eqsolver::variable_print<eqsolver::grid_storage_bits>;
		vprinter_t vprinter(dim.height(), dim.width(),
			p.x, p.y,
			arr, helper_vars);
		eqsolver::ast_print<vprinter_t> solver(&vprinter);
		return solver(ast);
	}

public:
	int border_width() const { return _border_width; }
	n_t get_neighbourhood() const
	{
		int bw = border_width();
		unsigned n_width = (bw<<1) + 1;
		dimension moore = { n_width, n_width };
		return n_t(moore, point(bw, bw));
	}
	//bool can_optimize_table() const { return num_states }
};

class bit_reference_base
{
protected:
	uint64_t minbit, bitmask;
	bit_reference_base(uint64_t minbit, uint64_t bitmask) :
		minbit(minbit),
		bitmask(bitmask) {}
	cell_t bits_in(const uint64_t& grid) const {
		return (grid >> minbit) & bitmask;
	}
};

class const_bit_reference : public bit_reference_base
{
	const uint64_t& grid;
public:
	const_bit_reference(const uint64_t& grid, uint64_t minbit, uint64_t bitmask) :
		bit_reference_base(minbit, bitmask),
		grid(grid)
	{
	}

public:
	operator cell_t () const { return bits_in(grid); }
};

class bit_reference : public bit_reference_base
{
// TODO:
//	friend class bitgrid_t; // TODO?
//	reference();	//! no public constructor
	//u_coord_t minbit;

	uint64_t& grid;
public:
	bit_reference(uint64_t& grid, uint64_t minbit, uint64_t bitmask) :
		bit_reference_base(minbit, bitmask),
		grid(grid)
	{
	}
public:
	operator cell_t () const { return bits_in(grid); }

	//! TODO: faster ops for ++ etc.
	bit_reference& operator= (const cell_t c) {
		grid = grid & (~(bitmask << minbit));
		grid = grid | ((uint64_t)c << minbit);
		return *this;
	}
	bit_reference& operator++() {
		grid += (uint64_t)(1 << minbit);
		return *this;
	}
};


class bitcell_itr_base
{
protected:
	using storage_t = uint64_t;
	const storage_t each;
	coord_t linewidth;
	//cell_t *ptr, *next_line_end;
	u_coord_t ptr, next_line_end;
	coord_t bw_2;
	const storage_t bitmask;

public:
	bitcell_itr_base(storage_t each, dimension dim, coord_t bw,
		bool pos_is_begin = true) :
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

	bitcell_itr_base& operator++() // TODO: the return value of this is a bug
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

	bool operator==(const bitcell_itr_base& rhs) const {
		return ptr == rhs.ptr; }
	bool operator!=(const bitcell_itr_base& rhs) const {
		return !operator==(rhs); }
};

class const_bitcell_itr : public bitcell_itr_base
{
	storage_t grid;
public:
	const_bitcell_itr(storage_t grid, storage_t each, dimension dim, coord_t bw,
		bool pos_is_begin = true) :
		bitcell_itr_base(each, dim, bw, pos_is_begin),
		grid(grid) {}

	const_bit_reference operator*() const {
		return const_bit_reference(grid, ptr * each, bitmask);
	}
};

class bitcell_itr : public bitcell_itr_base
{
	storage_t& grid;
public:
	bitcell_itr(storage_t& grid, storage_t each, dimension dim, coord_t bw,
		bool pos_is_begin = true) :
		bitcell_itr_base(each, dim, bw, pos_is_begin),
		grid(grid) {}

	bit_reference operator*() {
		return bit_reference(grid, ptr * each, bitmask);
	}
};

class bitgrid_t : public grid_alignment_t
{
/*	std::vector<cell_t> _data;
	dimension _dim; //! dimension of data, including borders
	u_coord_t bw, bw_2;*/


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

	bit_reference operator[](point p)
	{
		//return (grid >> (index(p)*each)) & bitmask;
		return bit_reference(grid, index(p)*each, bitmask);
	}

	const const_bit_reference operator[](point p) const
	{
		return const_bit_reference(grid, index(p)*each, bitmask);
		//return (grid >> (index(p)*each)) & bitmask;
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const bitgrid_t& g) {
		const bit_storage_w str(g.grid, g.each);
		const number_grid ng{};
		write_grid(&ng, stream, g._dim, g.bw, str);
		return stream;
	}

	//TODO: reference class like in std::vector<bool>?
//	cell_t get(point p) { return (grid >> (index(p)*each)) & bitmask; }
//	void set(point p, const cell_t& value) { return grid = grid & (bitmask << (index(p)*each)); }

	bitcell_itr begin() { return bitcell_itr(grid, each, _dim, bw); }
	bitcell_itr end() { return bitcell_itr(grid, each, _dim, bw, false); }
	const_bitcell_itr cbegin() const { return const_bitcell_itr(grid, each, _dim, bw); }
	const_bitcell_itr cend() const { return const_bitcell_itr(grid, each, _dim, bw, false); }

	storage_t raw_value() const { return grid; }
};


class ca_table_t : public ca_eqsolver_t
{
	unsigned n_w, size_each;
	point center;

	class size_check
	{
		size_check(int size)
		{
			if(size < 0)
			 throw "Error: Size negative (did you set num_states == 0?).";
			if(size > (1<<18))
			 throw "Error: This ca is too large for a table.";
		}
	};

	std::vector<uint64_t> table;

	using storage_t = uint64_t;

//protected: // TODO
public:
	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	ca_table_t(const char* equation, cell_t num_states = 0) :
		ca_eqsolver_t(equation, num_states),
		n_w((border_width()<<1) + 1),
		size_each((int)ceil(log(num_states))), // TODO: use int arithm
		center(border_width(), border_width()),
		table(1 << (size_each * n_w * n_w))
	{
		bitgrid_t grid(size_each, dimension(n_w, n_w), 0, 0);
		const dimension& dim = grid.internal_dim();
		std::size_t max = (int)pow(num_states, (n_w * n_w));
	//	std::cout << "max: " << max << std::endl;
	//	std::cout << "table size: " << table.size() << std::endl;
	//	std::cout << "size each: " << size_each << std::endl;

		std::size_t percent = 0, cur;

		std::cout << "Precalculating table, please wait..." << std::endl;
		// odometer
		for(std::size_t i = 0; i < max; ++i)
		{
			// evaluate
			// std::cout << "at " << grid.raw_value() ;
			table.at(grid.raw_value()) = ca_eqsolver_t::
				ca_eqsolver_t::
				calculate_next_state(grid.raw_value(), size_each, center, dim);
			// std::cout << ": " << table.at(grid.raw_value()) << std::endl;

			cur = (i * 100) / max; // max can never be 0
			if(percent < cur)
			{
				percent = cur;
				std::cout << "..." << percent << " percent" << std::endl;
			}

			// increase
			{
				bool go_on = true;
				for(bitcell_itr itr = grid.begin(); itr != grid.end() && go_on; ++itr)
				{
					go_on = ((*itr = (((*itr) + 1) % num_states)) == 0);
				}
			}
		}
	}

	//int calculate_next_state(const grid_t& grid_ptr,
	//	const point& p) const
	int calculate_next_state(const int *cell_ptr,
		const point& p, const dimension& dim) const
	{
		// TODO: class member?

		(void)p; // for a ca, the coordinates are no cell input
		bitgrid_t bitgrid(size_each, dimension(n_w, n_w), 0);

		int min = INT_MAX, max = INT_MIN; // any better alternative?
		for(const point p2 : bitgrid.points())
		{
			//bitgrid[p2] = grid_ptr[p+p2-center];
			const point offs = p2 - center;
			const int* ptr = cell_ptr + offs.y * (coord_t)dim.width() + offs.x;
			bitgrid[p2] = *ptr;
			min = std::min(min, *ptr);
			max = std::max(max, *ptr);
		}
	//	std::cout << bitgrid << std::endl;
	//	std::cout << bitgrid.raw_value() << std::endl;

		// todo: better hashing function for not exactly n bits?
		const bool in_range = (min >= 0 && max < num_states);
		return in_range
			? table[bitgrid.raw_value()]
			: *cell_ptr; // can not happen, except for border -> don't change
	}

};


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

//! base class for all ca classes with virtual functions
class base
{
public:
#if 0
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
#endif
	struct default_asynchronicity
	{
		bool operator()(unsigned ) const { return sca_random::get_int(2); }
	};

	struct synchronous
	{
		bool operator()(unsigned ) const { return true; }
	};
#if 0
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

	virtual grid_t& grid() = 0;
	virtual const grid_t& grid() const = 0;
#endif
};

class input_array : public base
{
	//! Shall stabilize grid in one rush
	virtual void stabilize(const grid_t &grid) const = 0;
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
	using base = Solver;
public:
	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	_ca_calculator_t(const char* equation, unsigned num_states = 0) :
		Solver(equation, num_states)
	{
	}

	//! calculates next state at (human) position (x,y)
	//! @param dim the grids internal dimension
	int next_state(const int *cell_ptr, const point& p, const dimension& dim) const
	{
		return base::calculate_next_state(cell_ptr, p, dim);
	}

	//! overload, with x and y in internal format. slower.
	int next_state_realxy(const int *cell_ptr, const point& p, const dimension& dim) const
	{
		int bw = base::border_width();
		return next_state(cell_ptr, p - point { bw, bw }, dim);
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
		cell_t next = next_state(cell_ptr, p, grid.internal_dim());
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

class ca_simulator_t : private ca_calculator_t, public base
{
	grid_t _grid[2];
	grid_t *old_grid = _grid, *new_grid = _grid;
	n_t neighbours;
	std::vector<point> //recent_active_cells(old_grid->size()),
			new_changed_cells; // TODO: this vector will shrink :/
	std::set<point> cells_to_check; // TODO: use pointers here, like in grid
	int round = 0;
	bool async;
public:
	ca_simulator_t(const char* equation, unsigned num_states, bool async = false) :
		ca_calculator_t(equation, num_states),
		_grid{border_width(), border_width()},
		neighbours(get_neighbourhood()),
		async(async)
	{
	}

	grid_t& grid() { return *new_grid; }
	const grid_t& grid() const { return *new_grid; }

	//! prepares the ca to run only on cells from @a sim_rect
	void finalize(const rect& sim_rect)
	{
		_grid[1] = _grid[0]; // fit borders

		// make all cells active, but not those close to the border
		// TODO: make this generic for arbitrary neighbourhoods
		new_changed_cells.reserve(sim_rect.area());
		for( const point &p : sim_rect ) {
			new_changed_cells.push_back(p); }
	}

	//! prepares the ca
	void finalize() { finalize(_grid->internal_dim()); }

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
		for(const point& np : neighbours)
		{
			const point p = ap + np;
			if(sim_rect.is_inside(p))
		//	if(!_grid->point_is_on_border(p))
			 cells_to_check.insert(p);
		}
		new_changed_cells.clear();

		for(const point& p : cells_to_check )
		if(async(2))
		// TODO: use bool async template here to increase speed?
		// plus: exploit code duplication?
		{
			int new_value;
			const int old_value = (*old_grid)[p];
			(*new_grid)[p] = (new_value
				= next_state(&((*old_grid)[p]), p, _grid->internal_dim()));
			if(new_value != old_value)
			{
				new_changed_cells.push_back(p);
			}

		}
		else
		{	// i.e. async + not activated
			// we still need to assign the old value:
			(*new_grid)[p] = (*old_grid)[p];
		}

		++round;
	}

	const std::vector<point>& active_cells() const { return new_changed_cells; }
	bool has_active_cells() const { return active_cells().empty(); }

	//! runs the whole ca
	template<class Asynchronicity>
	void _run_once(const Asynchronicity& async = synchronous())
	{
		run_once(rect(_grid->internal_dim(), border_width()), async);
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
};

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

} }

#endif // CA_H
