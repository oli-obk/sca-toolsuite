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

#include "io/secfile.h"
#include "ca.h"
#include "ca_table.h"
#include "utils/name_type_map.h"

namespace sca {

namespace grid_io {

struct grid_pair_t
{
	grid_t in, out;
public:
	friend std::istream& operator>> (std::istream& stream,
		grid_pair_t& gp) {
		return stream >> gp.in >> gp.out;
	}
};
/*
class symmetry_t : public io::supersection_t
{
	class rotate_t {};
	class mirror_t {};
public:
	symmetry_t() : supersection_t(type_t::sections) {
		init_leaf<io::leaf_template_t<void>>("rotate");
		init_leaf<io::leaf_template_t<void>>("mirror");
	}
};*/

class grid_group_t : public io::supersection_t
{
public:
	grid_group_t() : supersection_t(type_t::batch) {
//		init_subsection<symmetry_t>("symmetry");

		init_leaf<io::leaf_template_t<void>>("rotate");
		init_leaf<io::leaf_template_t<void>>("mirror");

		init_factory<io::leaf_template_t<grid_pair_t>>();
		set_batch_str("t");
	}
};


class gridfile_t : public io::supersection_t // TODO: public?
{
public:
	gridfile_t() : supersection_t(type_t::batch)
	{
		init_factory<grid_group_t>();
		set_batch_str("group");

		init_leaf<io::leaf_template_t<grid_t>>("n_in");
		init_leaf<io::leaf_template_t<grid_t>>("n_out");
		init_leaf<io::leaf_template_t<grid_t>>("center");
	}
};

}

namespace io
{

template<> // TODO: abstract case of path_node? enable_if?
class leaf_template_t<grid_io::grid_pair_t> : public leaf_base_t // TODO: why is this necessary?
{
	grid_io::grid_pair_t t;
public:
	void parse(io::secfile_t& inf) { inf.stream >> t; }
	void dump(std::ostream& ) const { throw "not impl"; }
	const grid_io::grid_pair_t& value() const noexcept { return t; }
	grid_io::grid_pair_t& value() noexcept { return t; }
};

}

namespace ca {

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

	bool index_ok(int idx, bool _rot, bool _mirr)
	{
		return (_mirr || !(idx&4)) && (_rot || !(idx&3));
	}

	template<class Traits, class CellTraits>
	void add_transition_functions(
		const _grid_t<Traits, CellTraits>& input_grid,
		const _grid_t<Traits, CellTraits>& output_grid,
		const point& p_in,
		const point& p_out,
		bool _rot,
		bool _mirr)
	{
		// family of 8 trans functions, subgroup of D4
	//	assert(bb.x_size() == input_grid.dim().width());
	//	assert(bb.y_size() == input_grid.dim().height());

		// TODO: unroll?
		int used = 0;
		for(int i = 0; i < 8; ++i)
		if(index_ok(i, _rot, _mirr))
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
		center_in(n_in.center()), // TODO: remove them all
		center_out(n_in.center()),
		n_dim(n_in.dim()),
		rect_in(n_in.get_rect()),
		rect_out(n_out.get_rect()),
		rect_max(rect_cover(rect_in, rect_out)),
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

	void add(const grid_t& in_grid, const grid_t& out_grid,
		bool _rot, bool _mirr)
	{
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
				 add_transition_functions(in_grid, out_grid, p_in, p_in, _rot, _mirr);
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
				 add_transition_functions(in_grid, out_grid, *itr_in, *itr_out, _rot, _mirr);
			}
				break;
			default: {
				throw "In and out grid must be equal-sized or `fit' each other.";
			}
		}

	}
};


class trans_vector_t
{
	ca::n_t n_in, n_out;
	using table_v = std::vector<ca::trans_t>;
	table_v table; //!< sorted by input (first)

	static table_v get_table_from_cons(table_v&& _tbl)
	{
		assert(_tbl.size() > 0);

		// uniq assertion
		std::sort(_tbl.begin(), _tbl.end(), ca::compare_by_input);
		const ca::trans_t* recent = &(_tbl[0]);

		for(table_v::const_iterator itr
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

	static trans_vector_t from_stream(std::istream& in = std::cin)
	{
		point center;
		std::size_t center_count = 0;

		io::secfile_t inf(in);
		grid_io::gridfile_t gridfile;

		try {
			gridfile.parse(inf);
		} catch(io::secfile_t::error_t ife) {
			std::cerr << "infile line " << ife.line << ": "	 << ife.msg << std::endl;
		}


		const grid_t& center_grid = gridfile.value<grid_t>("center");
		for(const point& p : center_grid.points())
		if(center_grid[p])
		{
			center = p;
			++center_count;
		}

		if(center_count != 1)
		 throw "Expected exactly 1 center cell.";

		ca::n_t n_in(gridfile.value<grid_t>("n_in"), center);
		ca::n_t n_out(gridfile.value<grid_t>("n_out"), center);

		tv_ctor cons(n_in, n_out);
		for(std::size_t i = 0; i < gridfile.max(); ++i)
		{
			const io::supersection_t& g = gridfile[i];
			bool rot = g.leaf<void>("rotate").is_read();
			bool mirr = g.leaf<void>("mirror").is_read();
			for(std::size_t j = 0; j < g.max(); ++j)
			{
				const grid_io::grid_pair_t& gp = g.value<grid_io::grid_pair_t>(j);
				cons.add(gp.in, gp.out, rot, mirr);
			}
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

	void dump_as_table(std::ostream& stream) const
	{
		// TODO: count num of states?
		table_t t(table, 3, convert<bitgrid_traits>(n_in),
			convert<bitgrid_traits>(n_out));
		t.dump(stream);
	}
};

enum class type
{
	formula,
	table,
	grids,
	invalid
};


util::name_type_map_t<3, type, type::invalid> name_type_map = {{
	{ "formula", type::formula },
	{ "table", type::table },
	{ "grids", type::grids }
}};


class converter_base {
protected:
	using in_t = std::istream&;
	using out_t = std::ostream&;
};

template<type in, type out>
class converter
{
	util::dont_instantiate_me_id<type, in> d;
};

template<type T>
struct converter<T, T>
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
		const trans_vector_t tv = trans_vector_t::
			from_stream(in);
		tv.dump_as_formula(out);
	}
};

template<>
struct converter<type::grids, type::table> : converter_base
{
	void operator()(in_t& in = std::cin, out_t& out = std::cout) const
	{
		const trans_vector_t tv = trans_vector_t::
			from_stream(in);
		tv.dump_as_table(out);
	}
};

template<>
struct converter<type::table, type::grids> : converter_base
{
	void operator()(in_t& in = std::cin, out_t& out = std::cout
		) const
	{
		// need to read the whole table for mirroring...
		// (mirroring is unimpl)
		const table_t tbl(in);
		tbl.dump_as_grids(out);
	}
};

template<>
struct converter<type::table, type::formula> : converter_base
{
	void operator()(in_t&  = std::cin, out_t&  = std::cout
		) const {
		throw "Sorry, unimplemented.\n"
			"Please use this order: table -> grids -> formula.";
	}
};

template<>
struct converter<type::formula, type::table> : converter_base
{
	void operator()(in_t&  = std::cin, out_t&  = std::cout
		) const {
		throw "Sorry, unimplemented.\n"
			"Coming soon...";
	}
};

template<>
struct converter<type::formula, type::grids> : converter_base
{
	void operator()(in_t&  = std::cin, out_t&  = std::cout
		) const {
		throw "Sorry, unimplemented.";
	}
};

template<type t_in, type t_out>
void convert(std::istream& in = std::cin,
	std::ostream& out = std::cout)
{
	converter<t_in, t_out> c; c(in, out);
}

template<type T1>
void _convert_dynamic(type t2, std::istream& in, std::ostream& out)
{
	switch(t2)
	{
		case type::formula:
			convert<T1, type::formula>(in, out); break;
		case type::table:
			convert<T1, type::table>(in, out); break;
		case type::grids:
			convert<T1, type::grids>(in, out); break;
		default: throw "invalid ca type";
	}
}

void convert_dynamic(type t1, type t2,
	std::istream& in = std::cin,
	std::ostream& out = std::cout)
{
	switch(t1)
	{
		case type::formula:
			_convert_dynamic<type::formula>(t2, in, out); break;
		case type::table:
			_convert_dynamic<type::table>(t2, in, out); break;
		case type::grids:
			_convert_dynamic<type::grids>(t2, in, out); break;
		default: throw "invalid ca type";
	}
}

void convert_dynamic(const char* t1, const char* t2,
	std::istream& in = std::cin,
	std::ostream& out = std::cout)
{
	convert_dynamic(name_type_map[t1], name_type_map[t2],
		in, out);
}


}}

#endif // CA_CONVERT_H
