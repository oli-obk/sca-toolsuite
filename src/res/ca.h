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

/**
 * @brief This class holds anything a cellular automaton's function
 * needs to know.
 *
 * Thus it contains no grid.
 */
class ca_calculator_t
{
	eqsolver::expression_ast ast;
	int* helper_vars = nullptr; //!< @todo: auto_ptr
	int helpers_size;
	int border_width;
public:
	~ca_calculator_t() { delete[] helper_vars; }

	// TODO: single funcs to initialize and make const?
	// aka: : ast(private_build_ast), ...
	ca_calculator_t(const char* equation)
	{
	//	debug("Building AST from equation...\n");
		eqsolver::build_tree(equation, &ast);

		eqsolver::ast_area<eqsolver::variable_area_grid>
			grid_solver;
		border_width = (int)grid_solver(ast);
	//	printf("Size of Moore Neighbourhood: %d\n",
	//		border_width);

		eqsolver::ast_area<eqsolver::variable_area_helpers>
			helpers_solver;
		helpers_size = (int)helpers_solver(ast) + 1;
	//	printf("Size of Helper Variable Array: %d\n",
	//		helpers_size);
		if(helpers_size > 0)
		 helper_vars = new int[helpers_size];
	}
	int get_border_width() const { return border_width; }

	//! calculates next state at (human) position (x,y)
	//! @param dim the grids internal dimension
	int next_state(const int *cell_ptr, const point& p, const dimension& dim) const
	{
		// TODO: replace &((*old_grid)[internal]) by old_value
		// and make old_value a ptr/ref?
		eqsolver::variable_print vprinter(dim.height(), dim.width(),
			p.x, p.y,
			cell_ptr, helper_vars);
		eqsolver::ast_print<eqsolver::variable_print> solver(&vprinter);
		return (int)solver(ast);
	}

	//! overload, with x and y in internal format. slower.
	int next_state_realxy(const int *cell_ptr, const point& p, const dimension& dim) const
	{
		return next_state(cell_ptr, p - point { border_width, border_width }, dim);
	}

	//! overload with human coordinates and reference to grid. slower.
	int next_state_gridptr(const grid_t &grid, const point& p) const
	{
		return next_state(&grid[p], p, grid.dim());
	}

	//! returns whether cell at point @a p is active.
	//! @a result is set to the result in all cases, if it is not nullptr
	// TODO: overloads
	bool is_cell_active(const grid_t& grid, const point& p, cell_t* result = nullptr) const
	{
		const int* cell_ptr = &grid[p];
		cell_t next = next_state(cell_ptr, p, grid.dim());
		if(result)
		 *result = next;
		return next != *cell_ptr;
	}

	n_t get_neighbourhood() const
	{
		unsigned moore_width = (border_width<<1) + 1;
		dimension moore = { moore_width, moore_width };
		return n_t(moore, point(border_width, border_width));
	}
};

class asm_synch_calculator_t
{
	int border_width;
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
	int next_state_gridptr(const grid_t &grid, const point& p) const
	{
		return next_state(&grid[p], p, grid.dim());
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
			*result = next_state(cell_ptr, p, grid.dim());
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

class ca_simulator_t : private ca_calculator_t
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
	ca_simulator_t(const char* equation, bool async = false) :
		ca_calculator_t(equation),
		_grid({get_border_width(), get_border_width()}),
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
	void finalize() { finalize(_grid->dim()); }

	// TODO: function run_once_async()

	struct default_asynchronicity
	{
		bool operator()(unsigned ) const { return sca_random::get_int(2); }
	};

	struct synchronous
	{
		bool operator()(unsigned ) const { return true; }
	};

	//! runs the ca once, but only ever activating cells from @a sim_rect
	template<class Asynchronicity>
	void run_once(const rect& sim_rect,
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
				= next_state(&((*old_grid)[p]), p, _grid->dim()));
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
	void run_once(const Asynchronicity& async = synchronous())
	{
		run_once(rect(_grid->dim(), get_border_width()), async);
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
