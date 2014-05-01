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

#ifndef CA_BASICS_H
#define CA_BASICS_H

#include <set>
#include <algorithm>
#include <array>
#include "geometry.h"

namespace sca { namespace ca {

/**
 * @brief This class holds input and output values of a
 * local transition function.
 *
 * Where these values are depends on a neighborhood class,
 * which s not a part of this struct.
 */
class trans_t
{
	// group parameters
	const unsigned neighbour_size; //! size of input_vals
	// differing parameters
	std::vector<int> input_vals; //!< function input values
	std::vector<bool> input_set; //!< is the value a dontcare?
	int input_count = 0; //!< number of set bits in input_set
	int output; //! function output value

public:
	trans_t(const unsigned _neighbour_size)
	:
		neighbour_size(_neighbour_size),
		input_vals(neighbour_size),
		input_set(neighbour_size, false)
	{
	}

	trans_t operator=(
		const trans_t& other)
	{
		assert(neighbour_size == other.neighbour_size);
		input_vals = other.input_vals;
		input_set = other.input_set;
		input_count = other.input_count;
		output = other.output;
		return *this;
	}

	trans_t(
		int _neighbour_size,
		int output_val
		) : // TODO: 2ctors + reuse
		neighbour_size(_neighbour_size),
		input_vals(neighbour_size),
		input_set(neighbour_size, false),
		output(output_val)
	{
	}

	void set_neighbour(int neighbour_id, int val)
	{
		input_vals[neighbour_id] = val;
		if(!input_set[neighbour_id])
		{
			input_set[neighbour_id] = true;
			++input_count;
		}
		else assert(false);
	}
	int get_output() const { return output; }
	bool input(int neighbour_id, int* result) {
		bool is_set = input_set[neighbour_id];
		if(is_set) *result = input_vals[neighbour_id];
		return is_set;
	}

	// compares lexicographical, returns results for operator<
	// a dontcare in front counts as a smaller value
	friend bool _compare_by_input(const trans_t& lhs,
		const trans_t& rhs)
	{
		// TODO: assert same neighbour size, grid size?
		for(unsigned i = 0; i<lhs.neighbour_size; ++i)
		{
			if(lhs.input_set[i] || rhs.input_set[i])
			{
				// if one of tem is not set,
				// the one which is not set is smaller
				if(!(lhs.input_set[i]&&rhs.input_set[i]))
				 return (int)lhs.input_set[i]
					> (int)rhs.input_set[i];
				else if(lhs.input_vals[i]
					!=rhs.input_vals[i])
				 return (lhs.input_vals[i]
					<rhs.input_vals[i]);
			}
		}
		return false; // if both are equal
	}

	bool operator<(const trans_t& rhs) const
	{
		return (output == rhs.output)
			? _compare_by_input(*this, rhs)
			: (output < rhs.output);
	}

	bool operator==(const trans_t& rhs) const
	{
		if(input_count != rhs.input_count)
		 return false; // shortening
		for(unsigned i = 0; i<neighbour_size; ++i)
		{
			if(input_set[i]&&rhs.input_set[i])
			{
				if(input_vals[i]!=rhs.input_vals[i])
				 return false;
			}
			else if(input_set[i]!=input_set[i])
			  return false; // one is not set
		}
		return true;
	}

	bool operator!=(const trans_t& rhs) const
	{
		return ! operator==(rhs);
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const trans_t& tf) {
		stream << "Transition function: (";
		for( unsigned i = 0; i < tf.neighbour_size; ++i)
		{
			if(tf.input_set[i])
			 stream << (tf.input_vals[i]) << ' ';
			else
			 stream << "- ";
		}
		stream << ") - > " << tf.output;
		return stream;
	}
};

inline bool compare_by_input(const trans_t& lhs,
		const trans_t& rhs)
{
	return _compare_by_input(lhs, rhs);
}

template<class Container>
class _n_t
{
protected:
	_n_t() {} // TODO: bad coding style
	constexpr _n_t(const Container& neighbours/*, const bounding_box& bb*/) :
		neighbours(neighbours)
	//	bb(bb)
	{}

	//! neighbour positions, relative to center cell
	//! @invariant The points are always sorted (linewise)
	Container neighbours;
	//! positive offset of center cell
	//! this cell is obviously needed
//	point center_cell = point(-1, -1);
//	dimension dim; 	//! < dimension of neighbours
//	bounding_box bb;

	point idx(int idx, int symm)
	{
		point p = operator[](idx);
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


	void add_single_tf(
		trans_t* tfs,
		const point& center_cell,
		const grid_t& input_grid,
		int output_val,
		int symm)
	{
		trans_t tf(size(), output_val);
		for(unsigned i = 0; i < size(); ++i) {

			//tf.set_neighbour(i, input_grid[bb.coords_to_id(idx(i, symm)/*+center_cell*/)]);
			tf.set_neighbour(i, input_grid[idx(i, symm) +center_cell/*- bb.ul()+center_cell*/]);
		}
		*tfs = tf; // TODO: redundant
	}

public:
	point get_center_cell()
	{
		bounding_box _bb;
		for(point& p : neighbours)
		{
			_bb.add_point( p );
		}
		return point(-_bb.ul().x, -_bb.ul().y);
	}

	point operator[](unsigned i) const { return neighbours[i]; }
	unsigned size() const { return neighbours.size(); }


	void add_transition_functions(
		std::vector<trans_t>& tf_vector,
		const point& center_cell,
		const grid_t& input_grid,
		int output_val)
	{
		// family of 8 trans functions, subgroup of D4
		static trans_t tfs[8]
		 = {size(), size(), size(), size(),
			size(), size(), size(), size()};
	//	assert(bb.x_size() == input_grid.dim().width());
	//	assert(bb.y_size() == input_grid.dim().height());

		// TODO: unroll?
		for(int i = 0; i < 8; ++i)
		 add_single_tf(tfs+i, center_cell, input_grid, output_val, i);

		std::sort(tfs, tfs+8);
		for(int i = 0; i < 8; ++i)
		 if((i==0) || (tfs[i-1] != tfs[i]))
		  tf_vector.push_back(tfs[i]);
	}

	/*neighbourhood(const dimension_container& _dim, point _center_cell = {0,0})
		: center_cell(_center_cell),
		dim({_dim.h, _dim.w})
	{
		neighbours.reserve(dim.area());
		for( const point& p : _dim )
		 neighbours.push_back(p);
	}*/

#if 0
	//! assumes that no borders exist
	_n_t(std::size_t point_no, const point* points, point _center_cell = {0,0})
		//: //center_cell(_center_cell),
		//dim(_dim)
	{
		/*neighbours.reserve(point_no);
		for( std::size_t i = 0; i < point_no; ++i )
		{
			neighbours.push_back(points[i] - _center_cell);
			bb.add_point(neighbours.back());
		}*/
	}
#endif

	bool contains(const point& p) const {
		for( const point& np : neighbours )
		 if( p == np ) // :)
		  return true;
		return false;
	}

	typedef std::vector<point>::const_iterator const_iterator;
	const_iterator begin() const { return neighbours.begin(); }
	const_iterator end() const { return neighbours.end(); }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }


public:
	// TODO: operator*
	_n_t& operator*=(const _n_t& rhs)
	{
		std::set<point> neighbour_set;

		for( const point& p : neighbours ) // TODO: const in all fors
		{
			for(unsigned i = 0; i < rhs.neighbours.size(); ++i)
			{
				neighbour_set.insert( p + rhs.neighbours[i] );
			}
		}

	/*	for( const point& np : neighbours ) // but erase points that are in our own cell
		 neighbour_set.erase(np);

		for( const point& p : neighbour_set )
		 neighbours.push_back(p);*/

		neighbours.assign(neighbour_set.begin(), neighbour_set.end());

		return *this;
	}

	_n_t& operator-=(const _n_t& rhs)
	{
		// we use a new set because erasing from a vector
		// causes reallocations
		std::set<point> n_set(neighbours.begin(), neighbours.end());
		for(const point& p : rhs.neighbours)
		 n_set.erase(p);
		neighbours.assign(n_set.begin(), n_set.end());

		return *this;
	}

	_n_t& operator+=(const point& rhs)
	{
		for(point& p : neighbours)
		 p += rhs;
		return *this;
	}

	// TODO: shouldn't this return std::set?
	_n_t& operator+(const point& rhs)
	{
		//return _n_t(*this) += rhs;
		_n_t tmp(*this);
		return tmp += rhs;
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const _n_t& n) {
		stream << "Neighbourhood: (";
		for( const point& p : n.neighbours) { stream << p << ", "; }
		stream << ")";
		return stream;
	}

/*	void shift(const point& p)
	{
		for(point& np : neighbours)
		 np += p;
	}*/

/*	neighbourhood(const std::vector<point>& _points, point _center_cell)
		: center_cell(_center_cell)
	{
		neighbours.reserve(_points.size());
		bounding_box bb;
		for( const point& p : _points )
		{
			point new_p = p - _center_cell;
			neighbours.push_back(new_p);
			bb.add_point(new_p);
		}
		dim = bb.dim();
	}*/
};

//using n_t = _n_t<std::vector<point>>;

class n_t : public _n_t<std::vector<point>>
{
	using base = _n_t<std::vector<point>>;
	void init(const std::vector<int>& in_grid,
		const dimension& in_dim)
	{
		point center_cell(-1, -1);
		for(unsigned y=0, id=0; y<in_dim.height(); ++y)
		for(unsigned x=0; x<in_dim.width(); ++x, ++id) // TODO: remove id
		{
			int elem = in_grid[id];
			switch(elem)
			{
			case 0:
				//printf("%d\n",center_cell.x);
				assert(center_cell.x < 0);
				center_cell.set(x, y);
				break;
			case 1:
			//	printf("%d\n",center_cell.x);
				assert(center_cell.x < 0);
				center_cell.set(x, y);
			case 2:
				base::neighbours.push_back(point(x,y));
				break;
			default: break;
			}
		}

		// make it all relative to center_cell
		for(point& p : neighbours)
		{
			// TODO: can be computed from dim?
			// TODO: but do not remove p-=...
			//base::bb.add_point( p -= center_cell );
			p -= center_cell;
		}
	}
public:
	/**
	 * @brief Reads neighborhood from grid.
	 *
	 * Cell values:
	 *  - 0 center cell which is *no* part of the nh.
	 *  - 1 center cell which *is* part of the nh.
	 *  - 2 nh cell
	 *  - 3 other cell (TODO: change: 2 grids of 0,1)
	 *
	 * @param in_grid
	 * @param in_dim
	 */
	n_t(const std::vector<int>& in_grid,
		const dimension& in_dim)
	{
		// TODO: parameter in_dim is useless?
		init(in_grid, in_dim);
	}

	// TODO: deprecated
	n_t(FILE* fp)
	{
		std::vector<int> in_grid;
		dimension tmp_dim;
		read_grid(fp, &in_grid, &tmp_dim, 0);
		init(in_grid, tmp_dim);
	}

	// TODO: make constexpr version, too
	//! assumes that no borders exist
	n_t(const dimension& _dim, point _center_cell = {0,0})
		//: //center_cell(_center_cell),
		//dim(_dim)
	{
		neighbours.reserve(_dim.area());
		dimension_container cont(_dim.height(), _dim.width(), 0);
		for( const point& p : cont )
		{
			neighbours.push_back(p - _center_cell);
			//bb.add_point(neighbours.back());
		}
	}
};

template<std::size_t N>
class n_t_constexpr : public _n_t<std::array<point, N>>
{
	using base = _n_t<std::array<point, N>>;
public:
	//! assumes that no borders exist
	constexpr n_t_constexpr(std::array<point, N> arr)
		: base(arr)
		//: //center_cell(_center_cell),
		//dim(_dim)
	{
		/*neighbours.reserve(point_no);
		for( std::size_t i = 0; i < point_no; ++i )
		{
			neighbours.push_back(points[i] - _center_cell);
			bb.add_point(neighbours.back());
		}*/
	}
};

class conf_t
{
	std::vector<cell_t> data; // TODO: list?
public:
/*	conf_t(const n_t& n, const grid_t& grid, point p = {0, 0})
	{
		for(unsigned i = 0; i < n.size(); ++i)
		 data.push_back(grid[p + n[i]]);
	}

	conf_t(const std::set<point>& points, const grid_t& grid, point p = {0, 0})
	{
		for(const point& p2 : points)
		 data.push_back(grid[p + p2]);
	}

	conf_t(const rect& points, const grid_t& grid, point p = {0, 0})
	{
		for(const point &p2 : points)
		 data.push_back(grid[p + p2]);
	}*/
	template<class Container>
	conf_t(const Container& points, const grid_t& grid, point p = {0, 0})
	{
		for(const point& p2 : points)
		 data.push_back(grid[p + p2]);
	}

	conf_t() {}

	bool equals_grid(const std::set<point>& points, const grid_t& grid) const
	{
		auto vitr = data.begin();
		for(auto itr = points.begin();
			itr != points.end();
			++itr, vitr++)
		 if(grid[*itr] != *vitr)
		  return false;
		return true;
	}

	bool operator<(const conf_t& rhs) const
	{
		unsigned size = data.size();
		assert(size == rhs.data.size());
		return data < rhs.data;
	}

	bool operator==(const conf_t& rhs) const
	{
		unsigned size = data.size();
		assert(size == rhs.data.size());
		return (data == rhs.data);
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const conf_t& c) {
		stream << "conf_tiguration: (";
		for( const cell_t& i : c.data) { stream << i << ", "; }
		stream << ")";
		return stream;
	}

	std::size_t size() const { return data.size(); }
	cell_t operator[](unsigned id) const { return data[id]; }

	//! @param pos position *before* which we should insert
	void insert_at_position(std::size_t pos, const cell_t& value)
	{
		assert(pos <= data.size());
		auto citr = data.begin();
		for( std::size_t cpos = 0; cpos != pos && citr != data.end()
			; ++citr, ++cpos ) ;
		data.insert(citr, value);
	}
};

template<class C1, class C2, class Selector>
bool for_each_selection(const C1& cont1, const C2& cont2, const Selector& sel)
{
	using itr1_t = typename C1::const_iterator;
	using itr2_t = typename C2::const_iterator;
	itr1_t itr1 = cont1.cbegin(), itr1_cp;
	itr2_t itr2 = cont2.cbegin(), itr2_cp;
	bool cont = true;
	for(; itr1 != cont1.cend() && itr2 != cont2.cend() && cont;)
	{
		if(*itr1 < *itr2)
		{
			itr1_cp = itr1++;
			cont = sel.first(itr1_cp);
		}
		else if(*itr2 < *itr1)
		{
			itr2_cp = itr2++;
			cont = sel.second(itr2_cp);
		}
		else {
			itr1_cp = itr1++;
			cont = sel.both(itr1_cp);
			++itr2;
		}
	}

	if(itr1 != cont1.cend() || itr2 != cont2.cend())
	{
		if(itr1 == cont1.cend())
		{
			// TODO: use while everywhere?
			for(; itr2 != cont2.cend() && cont; )
			{
				itr2_cp = itr2++;
				cont = sel.second(itr2_cp);
			}
		}
		else // i.e. itr2 at end
		{
			for(; itr1 != cont1.cend() && cont; )
			{
				itr1_cp = itr1++;
				cont = sel.first(itr1_cp);
			}
		}
	}
	return cont;
}

template<class Functor, class T1, class T2>
class base_selector
{
protected:
	const Functor& f;
public:
	bool both(const T1& ) const { return true; }
	bool first(const T1& ) const { return true; }
	bool second(const T2& ) const { return true; }
	base_selector(const Functor& f) : f(f) {}
};

template<class Functor, class T1, class T2>
struct selector_intersection : base_selector<Functor, T1, T2>
{
	typedef base_selector<Functor, T1, T2> base;
	using base::base_selector;
	bool both(const T1& t) const { return base::f(t); }
};

template<class Functor, class T1, class T2>
struct selector_first_only : base_selector<Functor, T1, T2>
{
	typedef base_selector<Functor, T1, T2> base;
	using base::base_selector;
	bool first(const T1& t) const { return base::f(t); }
};

template<class Functor, class T1, class T2>
struct selector_union : base_selector<Functor, T1, T2>
{
	typedef base_selector<Functor, T1, T2> base;
	using base::base_selector;
	// TODO: sfinae to call argument-less
	bool both(const T1& t) const { return base::f(t); }
	bool first(const T1& t) const { return base::f(t); }
	bool second(const T2& t) const { return base::f(t); }
};

template<class Functor1, class Functor2, class T1, class T2>
class selector_zip_base
	// (TODO): reuse selector base class with ellipsis?
{
	const Functor1& f1;
	const Functor2& f2;
public:
	bool both(const T1& ) const { assert(false); }
	bool first(const T1& t) const { return f1(t); }
	bool second(const T2& t) const { return f2(t); }
	selector_zip_base(const Functor1& f1, const Functor2& f2) :
		f1(f1), f2(f2) {}
};
/*
template<class C1, class C2>
void zip_to_vector(C1 cont1, C2 cont2)
{

}*/

template<class T>
class idx_iter
{
	typename T::const_iterator itr;
	std::size_t idx;
public:
	idx_iter& operator++() { ++itr; ++idx; return *this; }
	const typename T::value_type& operator*() const { return *itr; }
	std::size_t pos() const { return idx; }
};

template<class T>
class idx_container
{
	const T& cont;
public:
	idx_container(const T& cont) : cont(cont) {}
	idx_iter<T> cbegin() { return idx_iter<T>{ cont.cbegin(), 0 }; }
	idx_iter<T> cend() { return idx_iter<T>{ cont.cend(), cont.size() }; }
};

// TODO: all to geometry
template<template<class...> class Type, class C1, class C2, class ...Functor> // TODO: make union a template
bool for_each_points(const C1& cont1, const C2& cont2, const Functor&... func)
{
	return for_each_selection(cont1, cont2, Type<Functor..., decltype(cont1.begin()), decltype(cont2.begin())>(func...));
}

template<class C1, class C2, class Functor> // TODO: make union a template
bool for_ints_points(const C1& cont1, const C2& cont2, const Functor& func)
{
	return for_each_points<selector_intersection>(cont1, cont2, func);
}

template<class C1, class C2, class As1, class As2> // TODO: make union a template
auto zip_set(const C1& cont1, const C2& cont2, const As1& assoc1, const As2& assoc2)
-> std::set<std::size_t>
{
	using T = decltype(cont1.begin());
	std::set<T> result;
/*	const auto func1 = [&](const T& itr1) { result.push_back( assoc1[itr1 - cont1.begin()] ); };
	const auto func2 = [&](const T& itr2) { result.push_back( assoc2[itr2 - cont2.begin()] ); };
	for_each_selection<selector_zip_base>(cont1, cont2, func1, func2);*/
	return result;
}

/* // TODO
template<template<class, class> class Type, class R, class C1, class C2>
R points_make(const C1& cont1, const C2& cont2)
{
	R return_value;
	for_each_selection(cont1, cont2, Type<Functor, decltype(*cont1.begin())>(func));
	return return_value;
}*/

template<class C> void dump_container(const C& cont, std::ostream& stream = std::cout) {
	for(const auto& i : cont)
	 stream << i;
	stream << std::endl;
}

template<class C1, class C2>
bool is_subset_of(const C1& cont1, const C2& cont2)
{
	using T = decltype(cont1.begin());
	std::cout << "subset?" << std::endl;
	dump_container(cont1);
	dump_container(cont2);
	bool is_subset = for_each_points<selector_first_only>(cont1, cont2, [&](const T& ){ return false; });
	std::cout << is_subset << std::endl;
	return is_subset;
}

/*
template<class C1, class C2, class Functor>
void for_each_same(const C1& cont1, const C2& cont2, const Functor& func)
{
	const auto itr1 = cont1.begin();
	const auto itr2 = cont2.begin();
	for(; itr1 != cont1.end() && itr2 != cont2.end();)
	{
		if(*itr1 > *itr2)
		 ++itr2;
		else if(*itr2 > *itr1)
		 ++itr1;
		else {
			func(*itr1);
			++itr1;
			++itr2;
		}
	}
}
*/

} }

#endif // CA_BASICS_H
