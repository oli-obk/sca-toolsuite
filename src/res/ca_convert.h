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

//! @file ca_convert.h Routines to convert between ca formats

#ifndef CA_CONVERT_H
#define CA_CONVERT_H

#include "ca.h"

namespace sca { namespace ca {

class trans_vector_t;

class tv_ctor
{
	friend class trans_vector_t;

	const ca::n_t neighbours;
	const point center_cell;
	const dimension n_dim;
	grid_t _in_grid;
	std::vector<ca::trans_t> table; //!< unsorted
public:
	tv_ctor(const ca::n_t& neighbours) :
		neighbours(neighbours),
		center_cell(neighbours.get_center_cell()),
		n_dim(neighbours.get_dim()),
		_in_grid(n_dim, 0)
	{}

	void add_direct(const grid_t& in, const grid_t::value_type& out) {
		neighbours.add_transition_functions(
			table, center_cell, in, out
		);
	}

	void add_m2(const grid_t& in_grid, const grid_t& out_grid)
	{
		assert((2 + out_grid.human_dim().dx())
			== in_grid.human_dim().dx());
		assert((2 + out_grid.human_dim().dy())
			== in_grid.human_dim().dy());

		const point ul = *in_grid.points().end();
		const dimension rc(ul.x-2, ul.y-2);
		for(const point& p : rc)
		{
			in_grid.copy_to_int(_in_grid, n_dim + p);
			add_direct(_in_grid, out_grid[p]);
		}
	}

	void add_eq(const grid_t& in_grid, const grid_t& out_grid)
	{
		assert((out_grid.human_dim())
			== in_grid.human_dim());

		const point ul = *in_grid.points().end(), one(1, 1);
		const rect rc(one, ul-one);
		for(const point& p : rc)
		{
			in_grid.copy_to_int(_in_grid, n_dim + (p-point(1,1)));
			add_direct(_in_grid, out_grid[p]);
		}
	}
};

class trans_vector_t
{
	ca::n_t neighbours;
	using table_t = std::vector<ca::trans_t>;
	table_t table; //!< sorted by input (first)

	static table_t get_table_from_cons(table_t&& _tbl)
	{
		assert(_tbl.size() > 0);

		// uniq assertion
		std::sort(_tbl.begin(), _tbl.end(), ca::compare_by_input);
		const ca::trans_t* recent = &(_tbl[0]);

		for(table_t::const_iterator itr
			= (++_tbl.begin()); itr != _tbl.end(); ++itr)
		{
			assert(*itr != *recent);
			recent = &*itr;
		}

		return _tbl;
	}

public:
	trans_vector_t() {}
	trans_vector_t(tv_ctor&& cons) :
		neighbours(std::move(cons.neighbours)),
		table(get_table_from_cons(std::move(cons.table)))
	{}

	void dump_as_formula(std::ostream& stream) const
	{
		// write to out // TODO: redundant?
		std::vector<ca::trans_t> table_copy = table;
		std::sort(table_copy.begin(), table_copy.end()); // compares only by output

		{
			std::size_t braces = 0;

			// print helper vars
			for(std::size_t i = 0; i < neighbours.size(); i++)
			{
				point p = neighbours[i];
				//printf("h[%lu]:=a[%d,%d],\n",
				//	i, p.x, p.y);
				stream << "h[" << i << "]:=a["
					<< p.x << "," << p.y << "],\n";
			}

			int recent_output = table_copy[0].get_output();
			stream << "(" << std::endl;
			// print functions
			for(const auto& tf : table_copy)
			{
				if(recent_output != tf.get_output())
				{
					//printf("0 ) ? %d : (\n"
					//"(\n", recent_output);
					stream << "0 ) ? " << recent_output
						<< " : (\n(\n";
					++braces;
					recent_output = tf.get_output();
				}

				for(unsigned i = 0; i < neighbours.size(); i++)
				{
					int input_val;
					const bool is_set = tf.input(i, &input_val);
					assert(is_set);
					if(is_set)
						stream << "(h[" << i << "] == " << input_val
							<< ") && \n";
					// printf("(h[%d] == %d) && \n",
					//	i, input_val);
				}
				stream << " 1 ||" << std::endl;
			}
			// keep value as v if no matches
			//printf("0 ) ? %d : v", recent_output);
			stream << "0 ) ? " << recent_output << " : v";
			for(std::size_t i = 0; i < braces; ++i)
			 stream << ")";
			 //printf(")");
			stream << std::endl;
		}
	}

};

enum class type
{
	formula,
	table,
	grids
};

class converter_base {
protected:
	using in_t = std::istream&;
	using out_t = std::ostream&;
};

template<type in, type out>
class converter
{
	dont_instantiate_me_id<type, in> d;
};

template<type T>
class converter<T, T>
{
	void operator()(std::istream& i, std::ostream& o){
		o << i.rdbuf();
	}
};

/*template<>
class converter<type::table, type::grids>
{
	void operator()(std::istream& i, std::ostream& o) {
	//	_ca_table_hdr_t tbl_hdr(i);
	//	(void)tbl_hdr(i);
		dont_instantiate_me_id<type, in>();
	}
};*/

template<>
struct converter<type::grids, type::formula> : converter_base
{
	void operator()(in_t& in = std::cin, out_t& out = std::cout) const
	{
		ca::n_t neighbours(in);
		tv_ctor cons(neighbours);
		while(in.good())
		{
			const grid_t in_grid(in, 0);
			const grid_t out_grid(in, 0);
			cons.add_m2(in_grid, out_grid);
		}

		const trans_vector_t tv(std::move(cons));
		tv.dump_as_formula(out);
	}
};

}}

#endif // CA_CONVERT_H
