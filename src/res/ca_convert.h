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

	using u_coord_t = typename def_coord_traits::u_coord_t;

	const ca::n_t n_in, n_out;
	const point center_in, center_out;
	const dimension n_dim;
	const rect rect_in, rect_out;
	const rect rect_max;
	grid_t _in_grid;
	std::vector<ca::trans_t> table; //!< unsorted

	rect get_rect_max() const
	{
		return rect(point(std::min(rect_in.ul().x, rect_out.ul().x),
				std::min(rect_in.ul().y, rect_out.ul().y)),
			point(std::max(rect_in.lr().x, rect_out.lr().x),
				std::max(rect_in.lr().y, rect_out.lr().y)));
	}

	bool fits(const dimension& in_dim, const dimension& out_dim)
	{
		point vec(in_dim.dx(), in_dim.dy());
		vec -= point(rect_in.dx(), rect_in.dy()); // now vect is what we can read
		vec += point(rect_out.dx(), rect_out.dy()); // now vect is what we will write
		return (out_dim.dx() == (u_coord_t)vec.x)
			&& (out_dim.dy() == (u_coord_t)vec.y);
	}


	point idx(int idx, int symm, const n_t& n) const
	{
		point p = n[idx];
		const int rot = symm & 3;
		const int refl = symm >> 2;

		static const matrix rot_mat(0, -1, 1, 0);
		static const matrix rot_mats[] =
		 { matrix::id, rot_mat, rot_mat^2, rot_mat^3 };
		static const matrix mirror_mats[] =
		 { matrix::id, matrix(1, 0, 0, -1) };

		const point new_point
		= rot_mats[rot] * (mirror_mats[refl] * p);
		return new_point;
	}

	template<class Traits, class CellTraits>
	void add_single_tf(
		trans_t* tfs,
		const point& p_in,
		const point& p_out,
		const _grid_t<Traits, CellTraits>& input_grid,
		const _grid_t<Traits, CellTraits>& output_grid,
		int symm) const
	{
		trans_t tf(n_in.size(), n_out.size());
#ifdef SCA_DEBUG
		std::cerr << "Adding rotation: " << symm << std::endl;
#endif
		for(unsigned i = 0; i < n_in.size(); ++i) {
			// TODO: use center_in, center_out?
			//tf.set_neighbour(i, input_grid[bb.coords_to_id(idx(i, symm)/*+center_cell*/)]);
			std::cerr << idx(i, symm, n_in) << ", " << p_out << std::endl;
			tf.set_neighbour(i, input_grid[idx(i, symm, n_in) + p_in/*- bb.ul()+center_cell*/]);
		}

		for(unsigned i = 0; i < n_out.size(); ++i) {

			//tf.set_neighbour(i, input_grid[bb.coords_to_id(idx(i, symm)/*+center_cell*/)]);
		//	std::cout << idx(i, symm, n_out) << ", " << p_out << std::endl;
			tf.set_output(i, output_grid[idx(i, symm, n_out) + p_out/*- bb.ul()+center_cell*/]);
		}

		*tfs = tf; // TODO: redundant
	}

	//! family of 8 trans functions, subgroup of D4
	mutable trans_t tmp_tfs[8];

	bool index_ok(int idx, bool rot, bool mirr)
	{
		return (mirr || !(idx&4)) && (rot || !(idx&3));
	}

	template<class Traits, class CellTraits>
	void add_transition_functions(
		const _grid_t<Traits, CellTraits>& input_grid,
		const _grid_t<Traits, CellTraits>& output_grid,
		const point& p_in,
		const point& p_out,
		bool rot,
		bool mirr)
	{
		// family of 8 trans functions, subgroup of D4
	//	assert(bb.x_size() == input_grid.dim().width());
	//	assert(bb.y_size() == input_grid.dim().height());

		// TODO: unroll?
		int used = 0;
		for(int i = 0; i < 8; ++i)
		if(index_ok(i, rot, mirr))
		{
			++used;
			add_single_tf(tmp_tfs + i, p_in, p_out, input_grid, output_grid, i);
		}


		std::sort(tmp_tfs, tmp_tfs + used);
		for(int i = 0; i < used; ++i)
		if((i==0) || (tmp_tfs[i-1] != tmp_tfs[i]))
		{
#ifdef SCA_DEBUG
			std::cerr << "pushing back: " << tmp_tfs[i] << std::endl;
#endif
			table.push_back(tmp_tfs[i]);
		}
	}

public:
	tv_ctor(const ca::n_t& n_in, const ca::n_t& n_out) :
		n_in(n_in),
		n_out(n_out),
		center_in(n_in.get_center_cell()),
		center_out(n_in.get_center_cell()),
		n_dim(n_in.get_dim()),
		rect_in(n_in.get_rect()),
		rect_out(n_out.get_rect()),
		rect_max(get_rect_max()),
		_in_grid(n_dim, 0),
		tmp_tfs {
			trans_t(n_in.size(), n_out.size()),
			trans_t(n_in.size(), n_out.size()),
			trans_t(n_in.size(), n_out.size()),
			trans_t(n_in.size(), n_out.size()),
			trans_t(n_in.size(), n_out.size()),
			trans_t(n_in.size(), n_out.size()),
			trans_t(n_in.size(), n_out.size()),
			trans_t(n_in.size(), n_out.size()),
		}
	{
#ifdef SCA_DEBUG
		std::cerr << "N in: " << n_in << std::endl;
		std::cerr << "N out: " << n_out << std::endl;
#endif
	}

	void add(const grid_t& in_grid, const grid_t& out_grid, bool rot, bool mirr)
	{
		// TODO: read rot, mirr from file

		enum class mode_t
		{
			equal,
			fit,
			invalid
		};

		mode_t mode = (in_grid.human_dim() == out_grid.human_dim())
			? mode_t::equal
			: fits(in_grid.human_dim(), out_grid.human_dim())
				? mode_t::fit
				: mode_t::invalid;

		switch(mode)
		{
			case mode_t::equal:
			{
				point in_max(in_grid.human_rect().lr());
				rect rc(- rect_max.ul(), in_max - rect_max.lr());
#ifdef SCA_DEBUG
				std::cerr << "Scanning rect: " << rc << std::endl;
#endif
				for(const point& p_in : rc)
				 add_transition_functions(in_grid, out_grid, p_in, p_in, rot, mirr);
			}
				break;
			case mode_t::fit:
			{
				point in_max(in_grid.human_rect().lr());
				rect rc_in(-rect_in.ul(), in_max - rect_in.lr());
				rect rc_out(-rect_out.ul(), in_max - rect_out.lr());

				auto itr_in = rc_in.begin();
				auto itr_out = rc_out.begin();

				for( ; itr_in != rc_in.end(); ++itr_in, ++itr_out)
				 add_transition_functions(in_grid, out_grid, *itr_in, *itr_out, rot, mirr);
			}
				break;
			default:
			 throw "In and out grid must be equal-sized or `fit' each other.";
		}

	}
};

class trans_vector_t
{
	ca::n_t n_in, n_out;
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
		n_in(std::move(cons.n_in)),
		n_out(std::move(cons.n_out)),
		table(get_table_from_cons(std::move(cons.table)))
	{}

	static trans_vector_t from_stream(std::istream& in = std::cin,
		bool rot = true, bool mirr = true)
	{
		point center;
		std::size_t center_count = 0;
		grid_t center_grid(in, 0);
		for(const point& p : center_grid.points())
		if(center_grid[p])
		{
			center = p;
			++center_count;
		}

		if(center_count != 1)
		 throw "Expected exactly 1 center cell.";

		grid_t center_grid_2(in, 0);
	//	in >> center_grid; // TODO: bug
	//	std::cout <<  center_grid << std::endl;
		ca::n_t n_in(center_grid_2, center);

	//	in >> center_grid;
	//	std::cout <<  center_grid << std::endl;
		grid_t center_grid_3(in, 0);
		ca::n_t n_out(center_grid_3, center);

		tv_ctor cons(n_in, n_out);
		while(in.good())
		{
			const grid_t in_grid(in, 0);
			const grid_t out_grid(in, 0);
			cons.add(in_grid, out_grid, rot, mirr);
		}

		return trans_vector_t(std::move(cons));
	}

	void dump_as_formula_at(std::ostream& stream, int output_idx) const
	{
		// write to out // TODO: redundant?
		std::vector<ca::trans_t> table_copy = table;
		std::sort(table_copy.begin(), table_copy.end()); // compares only by output

		{
			std::size_t braces = 0;

			int recent_output = table_copy[0].get_output()[output_idx];
			stream << "(" << std::endl;
			// print functions
			for(const auto& tf : table_copy)
			{
				if(recent_output != tf.get_output()[output_idx])
				{
					//printf("0 ) ? %d : (\n"
					//"(\n", recent_output);
					stream << "0 ) ? " << recent_output
						<< " : (\n(\n";
					++braces;
					recent_output = tf.get_output()[output_idx];
				}

				for(unsigned i = 0; i < n_in.size(); i++)
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

	void dump_as_formula(std::ostream& stream) const
	{
		// print helper vars
		for(std::size_t i = 0; i < n_in.size(); i++)
		{
			point p = n_in[i];
			//printf("h[%lu]:=a[%d,%d],\n",
			//	i, p.x, p.y);
			stream << "h[" << i << "]:=a["
				<< p.x << "," << p.y << "],\n";
		}

		for(const auto& _p : ca::counted(n_out))
		{
			const point& p = _p;
			stream << "(a[" << p.x << "," << p.y << "]:=" << std::endl;
			dump_as_formula_at(stream, _p.id());
			stream << ")," << std::endl;
		}

		stream << "v";
	}

	void dump_as_grids(std::ostream& stream) const
	{

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
	void operator()(in_t& in = std::cin, out_t& out = std::cout,
		bool rot = true, bool mirr = true) const
	{
		const trans_vector_t tv = trans_vector_t::
			from_stream(in, rot, mirr);
		tv.dump_as_formula(out);
	}
};

template<>
struct converter<type::grids, type::table> : converter_base
{
	void operator()(in_t& in = std::cin, out_t& out = std::cout,
		bool rot = true, bool mirr = true) const
	{
		const trans_vector_t tv = trans_vector_t::
			from_stream(in, rot, mirr);
		tv.dump_as_formula(out);
	}
};

}}

#endif // CA_CONVERT_H
