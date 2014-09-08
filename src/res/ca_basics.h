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
#include <type_traits>
#include "grid.h" // TODO: make grid a template?

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
	bool input(int neighbour_id, int* result) const {
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

// TODO: make a utils.h
template<typename ...> struct falsify : std::false_type { };
template<typename T, T Arg> class falsify_id : std::false_type { };
template<typename ...Args>
class dont_instantiate_me {
	static_assert(falsify<Args...>::value, "This should not be instantiated.");
};
template<typename T, T Arg>
class dont_instantiate_me_id {
	static_assert(falsify_id<T, Arg>::value, "This should not be instantiated.");
};

// TODO: std::array
template<class T>
std::size_t get_pos(const T&, typename T::const_reference) {
	return dont_instantiate_me<T>(), 0;
}

template<class VT>
std::size_t get_pos(const std::vector<VT>& cont, const VT& elem) {
	const auto& itr = std::lower_bound(cont.begin(), cont.end(), elem);
	if(itr == cont.end())
	 throw "Point not found";
	return std::distance(cont.begin(), itr);
}



template<class Traits, class Container>
class _n_t
{
	using point = _point<Traits>;
	using dimension = _dimension<Traits>;
protected:
#if 0
	_n_t() {} // TODO: bad coding style
	constexpr _n_t(const Container& neighbours/*, const bounding_box& bb*/) :
		neighbours(neighbours)
	//	bb(bb)
	{}
#endif
	//! neighbour positions, relative to center cell
	//! @invariant The points are always sorted (linewise)
	Container neighbours;
	//! positive offset of center cell
	//! this cell is obviously needed

	point idx(int idx, int symm) const
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

	template<class CellTraits>
	void add_single_tf(
		trans_t* tfs,
		const point& center_cell,
		const _grid_t<Traits, CellTraits>& input_grid,
		int output_val,
		int symm) const
	{
		trans_t tf(size(), output_val);
		for(unsigned i = 0; i < size(); ++i) {

			//tf.set_neighbour(i, input_grid[bb.coords_to_id(idx(i, symm)/*+center_cell*/)]);
			tf.set_neighbour(i, input_grid[idx(i, symm) + center_cell/*- bb.ul()+center_cell*/]);
		}
		*tfs = tf; // TODO: redundant
	}

public:
	std::size_t pos(const point& p) const
	{
		return get_pos(neighbours, p);
	}

	point get_center_cell() const
	{
		_bounding_box<Traits> _bb;
		for(const point& p : neighbours)
		 _bb.add_point( p );
		return point(-_bb.ul().x, -_bb.ul().y);
	}

	dimension get_dim() const
	{
		_bounding_box<Traits> _bb;
		for(const point& p : neighbours)
		 _bb.add_point( p );
		return _bb.dim();
	}

	point operator[](unsigned i) const { return neighbours[i]; }
	unsigned size() const { return neighbours.size(); }

	template<class CellTraits>
	void add_transition_functions(
		std::vector<trans_t>& tf_vector,
		const point& center_cell,
		const _grid_t<Traits, CellTraits>& input_grid,
		int output_val) const
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

	/*neighbourhood(const dim_cont& _dim, point _center_cell = {0,0})
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


	typedef typename Container::const_iterator const_iterator;
	const_iterator begin() const { return neighbours.begin(); }
	const_iterator end() const { return neighbours.end(); }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }


public:
	_n_t operator*(const _n_t& rhs) const
	{
		_n_t tmp(*this);
		tmp *= rhs;
		return tmp;
	}

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

	template<class Point>
	_n_t& operator+=(const Point& rhs)
	{
		for(point& p : neighbours)
		 p += rhs;
		return *this;
	}

	// TODO: shouldn't this return std::set?
	_n_t& operator+(const point& rhs) const
	{
		//return _n_t(*this) += rhs;
		_n_t tmp(*this);
		return tmp += rhs;
	}

	//! complexity: O(neighbours)
	template<class T>
	std::vector<_point<T>> operator()(const _point<T>& rhs) const
	{
		std::vector<_point<T>> result;
		result.reserve(neighbours.size());
		for(const point& np : neighbours)
		 result.push_back(rhs + _point<T>(np.x, np.y));
		return result;
	}

	//! complexity: theoretically O(neighbours), but we implemented it too slow
	template<class Cont>
	std::set<typename Cont::value_type> operator()(const Cont& rhs) const
	{
		using point_type = typename Cont::value_type;
		std::set<point_type> result;
		for(const point_type& p : rhs)
		for(const point& np : neighbours)
		 result.insert(p + point_type(np.x, np.y));
		return result;
	}

	template<class Functor>
	void for_each(const point& p, const Functor& ftor) const
	{
		for(const point& np : neighbours)
		 ftor(p + np);
	}

	template<class Functor>
	bool for_each_bool(const point& p, const Functor& ftor) const
	{
		bool ok = true;
		for(auto itr = neighbours.cbegin(); itr != neighbours.cend() && ok; ++itr)
		 ok = ok && ftor(p + *itr);
		return ok;
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const _n_t& n) {
		stream << "Neighbourhood: (";
		for( const point& p : n.neighbours) { stream << p << ", "; }
		stream << ")";
		return stream;
	}

	friend std::istream& operator>> (std::istream& stream,
		_n_t& n)
	{
		grid_t in_grid(stream, 0);
		n.init(in_grid.data(), in_grid.internal_dim());
		return stream;
	}

	bool is_neighbour_of(const point& p1, const point& p2) const
	{
		return std::binary_search(neighbours.begin(), neighbours.end(), p1-p2);
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
/*};

//using n_t = _n_t<std::vector<point>>;

template<class T> // TODO: unite with upper class...
class n_t_2 : public _n_t<T>
{
	using base = _n_t<T>;*/
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
				neighbours.push_back(point(x,y));
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
	_n_t(const std::vector<int>& in_grid,
		const dimension& in_dim)
	{
		// TODO: parameter in_dim is useless?
		init(in_grid, in_dim);
	}
	_n_t(std::istream& stream) { stream >> *this; } // TODO: const ctor
	_n_t() {}

	// TODO: make constexpr version, too
	//! assumes that no borders exist
	_n_t(const dimension& _dim, point _center_cell = {0,0})
		//: //center_cell(_center_cell),
		//dim(_dim)
	{
		neighbours.reserve(_dim.area());
		_dim_cont<Traits> cont(_dim.height(), _dim.width(), 0);
		for( const point& p : cont )
		{
			neighbours.push_back(p - _center_cell);
			//bb.add_point(neighbours.back());
		}
	}

	_n_t(const Container&& cont) : neighbours(cont) {}

	//! assumes that no borders exist
	template<std::size_t N>
	constexpr _n_t(const std::array<point, N>& arr)
		: neighbours(arr)
	{
	}

	// TODO: maybe simply return the n_t instead of the array?
	// TODO: cpp-file?
	// TODO: more common neighbourhoods
	static constexpr std::array<point, 9> moore_1_2d()
	{
		return std::array<point, 9> {{
			{-1, -1}, {0, -1}, {1, -1},
			{-1, 0}, {0, 0}, {1, 0},
			{-1, 1}, {0, 1}, {1, 1}
		}};
	}

	static constexpr std::array<point, 5> neumann_1_2d()
	{
		return std::array<point, 5> {{
			{0, -1},
			{-1, 0}, {0, 0}, {1, 0},
			{0, 1},
		}};
	}
};

using n_t = _n_t<def_coord_traits, std::vector<point>>;
template<std::size_t N>
using n_t_constexpr = _n_t<def_coord_traits, std::array<point, N>>;

#if 0
template<class T>
class static_arr
{
	std::size_t size;
	const T* arr;
public:
	T* begin() { return arr; }
	T* end() { return arr + size; }
	static_arr(std::size_t size, const T* arr) :
		size(size),
		arr(arr) {}
};

class n_t_const : public _n_t<static_arr<point>>
{
	using base = _n_t<static_arr<point>>;
public:
	//! assumes that no borders exist
	n_t_const(std::size_t size, point* arr) :
		base(static_arr<point>(size,arr))
	{
	}
};
#endif

/*
template<std::size_t N>
class n_t_constexpr : public _n_t<std::array<point, N>>
{
	using base = _n_t<std::array<point, N>>;
public:
	//! assumes that no borders exist
	constexpr n_t_constexpr(std::array<point, N> arr)
		: base(arr)
	{
	}
};*/

//! container which redirects assignment to calling a functor
template<class T, class Functor>
class fun_itr : public std::iterator<std::output_iterator_tag, T>
{
	class fun_functor
	{
		const Functor& ftor;
	public:
		using self = fun_functor;
		const self& operator=(const T& elem) const { ftor(elem); return *this; }
		fun_functor(const Functor& ftor) : ftor(ftor) {}
	};

	fun_functor fn;

public:
	using self = fun_itr<T, Functor>;
	using value_type = T;
	const self& operator++() const { return *this; }
	const fun_functor& operator*() { return fn; }

	fun_itr(const Functor& ftor)
	: fn(ftor)
	{}
};

template<class T, class Functor>
fun_itr<T, Functor>
make_fun_itr(Functor&& ftor) {
	return fun_itr<T, Functor>(std::forward<Functor>(ftor));
}

template<class CellTraits> // TODO: traits is not needed
class _conf_t
{
	using cell_t = typename CellTraits::cell_t;
//	using grid_t = _grid_t<Traits>;
//	using point = _point<Traits>;
	std::vector<cell_t> _data; // TODO: list?
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
public:
	_conf_t(cell_t single_val) : _data{single_val} {}
	_conf_t(cell_t single_val, std::size_t n) : _data(n, single_val) {}
	_conf_t(std::vector<cell_t>&& _data) : _data(_data) {} // TODO: public?

// TODO: re-enable
#if 0
	//! Construct merged c
	//! Complexity: O(conf1) + O(conf2)
	template<class OrdCont1, class OrdCont2>
	static _conf_t
	merge(const OrdCont1& coords1, const _conf_t& conf1, const OrdCont2& coords2, const _conf_t& conf2)
	{
		std::size_t new_size = 0;
		std::set_union(coords1.begin(), coords1.end(), coords2.begin(), coords2.end(),
			make_fun_itr<point>([&](const point&){ ++new_size; }));
		std::vector<cell_t> new_data(new_size);

		auto itr1 = coords1.begin();
		auto itr2 = coords2.begin();
		auto vec1 = conf1._data.begin();
		auto vec2 = conf2._data.begin();
		auto itr_res = new_data.begin();

		const auto& cb_union = [&](const point& p)
		{
			const cell_t* new_val = nullptr;
			if(itr1 != coords1.end() && *itr1 == p) {
				++itr1;
				new_val = &*(vec1++);
			}
			if(itr2 != coords2.end() && *itr2 == p) {
				if(new_val != nullptr && *new_val != *vec2)
				 throw "Can not merge: different values on same point.";
				++itr2;
				new_val = &*(vec2++);
			}
			*(itr_res++) = (*new_val);
		};
		std::set_union(coords1.begin(), coords1.end(), coords2.begin(), coords2.end(),
			make_fun_itr<point>(cb_union));

		return _conf_t(std::move(new_data));
	}
#endif

// TODO erase?
#if 0
	//! Construct substracted c, with points which are not in coords2, but in coords 1
	//! Complexity: O(conf1) + O(conf2)
	template<class OrdCont1, class OrdCont2>
	static _conf_t
	substract(const OrdCont1& coords1, const _conf_t& conf1, const OrdCont2& coords2, const _conf_t& conf2)
	{
		std::size_t new_size = 0;
		std::set_difference(coords1.begin(), coords1.end(), coords2.begin(), coords2.end(),
			make_fun_itr<point>([&](const point&){ ++new_size; }));
		std::vector<cell_t> new_data(new_size);

		auto itr1 = coords1.begin();
		auto vec1 = conf1.data.begin();
		auto itr_res = new_data.begin();

		const auto& cb_diff = [&](const point& p)
		{
			const cell_t* new_val = nullptr;
			while(itr1 != coords1.end() && *itr1 != p) {
				++itr1; ++vec1;
			}
			if(itr1 != coords1.end())
			 *(itr_res++) = *vec1;
		};
		std::set_difference(coords1.begin(), coords1.end(), coords2.begin(), coords2.end(),
			make_fun_itr<point>(cb_diff));

		return _conf_t(std::move(new_data));
	}
#endif

	template<class Container, class Traits>
	_conf_t(const Container& points,
		const _grid_t<Traits, CellTraits>& grid,
		_point<Traits> p = {0, 0})
	{
		for(const _point<Traits>& p2 : points)
		 _data.push_back(grid[p + p2]);
	}

	_conf_t() {}

	template<class Traits>
	bool equals_grid(const std::set<_point<Traits>>& points,
		const _grid_t<Traits, CellTraits>& grid) const
	{
		auto vitr = _data.begin();
		for(auto itr = points.begin();
			itr != points.end();
			++itr, vitr++)
		 if(grid[*itr] != *vitr)
		  return false;
		return true;
	}

	bool operator<(const _conf_t& rhs) const
	{
		unsigned size = _data.size();
		assert(size == rhs._data.size());
		return _data < rhs._data;
	}

	bool operator==(const _conf_t& rhs) const
	{
		unsigned size = _data.size();
		if(size != rhs._data.size()) // TODO: remove for debug
		{
			std::cout << "sizes: " << size << ", " << rhs._data.size() << std::endl;
		}
		assert(size == rhs._data.size());

		return (_data == rhs._data);
	}

	bool operator!=(const _conf_t& rhs) const
	{
		return !operator==(rhs);
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const _conf_t& c) {
		std::size_t skipped = 0;
		stream << "conf: (";
		for(auto itr = c.cbegin(); itr != c.cend(); ++itr)
		{
			// TODO: more general?
			auto itr2 = itr;
			++itr2;
			if(itr2 != c.cend() && *itr2 == *itr)
			 ++skipped;
			else
			{
				if(skipped == 0)
					stream << *itr;
				else
				{
					stream << *itr
						<< "[" << (1+skipped) << "x]";
					skipped = 0;
				}

				if(itr2 != c.cend())
				 stream << ", ";
			}
		}
		stream << ")";
		return stream;
	}

	std::size_t size() const { return _data.size(); }
	// TODO: the following returns a reference for vector<bool>
	const cell_t& operator[](unsigned id) const { return _data[id]; }
	cell_t& operator[](unsigned id) { return _data[id]; }

	const std::vector<cell_t>& data() const { return _data; }
	std::vector<cell_t>& data() { return _data; }

	//! @param pos position *before* which we should insert
	void insert_at_position(std::size_t pos, const cell_t& value)
	{
		assert(pos <= _data.size());
		auto citr = _data.begin();
		for( std::size_t cpos = 0; cpos != pos && citr != _data.end()
			; ++citr, ++cpos ) ;
		_data.insert(citr, value);
	}

	using iterator = typename std::vector<cell_t>::iterator;
	using const_iterator = typename std::vector<cell_t>::const_iterator;
	iterator begin() { return _data.begin(); }
	iterator end() { return _data.end(); }
	const_iterator cbegin() const { return _data.cbegin(); }
	const_iterator cend() const { return _data.cend(); }

	void clear() { _data.clear(); }
};
using conf_t = _conf_t<def_cell_traits>;

template<class Itr>
struct count_elem
{
	int _id;
	Itr _itr;
	template<class SomeItr> friend class counted_itr;
public:
	using value_type = typename Itr::value_type;

	count_elem() {}
	count_elem(Itr _itr, int id) : _id(id), _itr(_itr) {}

	int id() const { return _id; }
	operator const value_type&() const { return *_itr; }
//	operator ValueType&() { return _elem; }
};

template<class Itr>
class counted_itr
{
	count_elem<Itr> elem;
public:
	counted_itr(Itr _itr, int _idx) : elem(_itr, _idx) {}
	counted_itr() {}

	counted_itr& operator++() { ++(elem._itr); ++elem._id; return *this; }
	counted_itr operator++(int) { counted_itr old = *this; operator++(); return old; }

	count_elem<Itr> operator*() const { return elem; }
//	count_elem<Itr> operator*() { return elem; }
	const count_elem<Itr>* operator->() const { return &elem; }
//	count_elem<Itr>* operator->() { return &elem; }

	bool operator==(const counted_itr& rhs) const { return elem._itr == rhs.elem._itr; }
	bool operator!=(const counted_itr& rhs) const { return !operator==(rhs); }
};

template<class Cont>
class _counted
{
	Cont& cont;
	using cont_itr = typename std::remove_reference<Cont>::type::iterator;
	using cont_citr = typename std::remove_reference<Cont>::type::const_iterator;
public:
	_counted(Cont& c) : cont(std::forward<Cont>(c)) {}

	using const_iterator = counted_itr<cont_citr>;
	using iterator = counted_itr<cont_itr>;
	using value_type = typename cont_itr::value_type;

	// TODO!! (bug?)
//	iterator begin() { return counted_itr<cont_itr>(cont.begin(), 0); }
//	iterator end() { return counted_itr<cont_itr>(cont.end(), cont.size()); }
	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }
	const_iterator cbegin() const { return counted_itr<cont_citr>(cont.cbegin(), 0); }
	const_iterator cend() const { return counted_itr<cont_citr>(cont.cend(), cont.size()); }
};

template<class Cont>
_counted<Cont> counted(Cont&& cont) {
	return _counted<Cont>(std::forward<Cont>(cont));
}


template<class C1, class C2, class Selector>
bool zip(const C1& cont1, const C2& cont2, const Selector& sel)
{
	using itr1_t = typename C1::const_iterator;
	using itr2_t = typename C2::const_iterator;
	using vt = typename C1::value_type;
	static_assert(std::is_same<vt, typename C1::value_type>(),
		"Need equal types in both containers.");
	itr1_t itr1 = cont1.cbegin(), itr1_cp;
	itr2_t itr2 = cont2.cbegin(), itr2_cp;
	bool cont = true;
	for(; itr1 != cont1.cend() && itr2 != cont2.cend() && cont;)
	{
		if((vt)*itr1 < (vt)*itr2)
		{
			itr1_cp = itr1++;
			cont = sel.first(itr1_cp);
		}
		else if((vt)*itr2 < (vt)*itr1)
		{
			itr2_cp = itr2++;
			cont = sel.second(itr2_cp);
		}
		else { // TODO: better if-else-structure?
			itr1_cp = itr1++;
			itr2_cp = itr2++;
			cont = sel.both(itr1_cp, itr2_cp);
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

struct nop
{
	void operator()(...) const {}
};

struct true_nop
{
	bool operator()(...) const { return true; }
};

template<class Functor1, class Functor2, class Functor3, class T1, class T2>
class fun_selector
{
protected:
	const Functor1& f1;
	const Functor2& f2;
	const Functor3& f3;
public:
	template<class Itr> bool first(const Itr& elem) const { return f1(elem); }
	template<class Itr> bool second(const Itr& elem) const { return f2(elem); }
	template<class Itr1, class Itr2>
	bool both(const Itr1& itr1, const Itr2& itr2) const { return f3(itr1, itr2); }
	fun_selector(const Functor1& f1, const Functor2& f2, const Functor3& f3) :
		f1(f1), f2(f2), f3(f3) {}
};

template<class F1, class F2, class F3, class T1, class T2>
class fun_selector_symm : public fun_selector<F1, F2, F3, T1, T2>
{
	using base = fun_selector<F1, F2, F3, T1, T2>;
public:
	template<class Itr1, class Itr2>
	bool both(const Itr1& itr1, const Itr2&) const { return base::f3(itr1); }
	using base::fun_selector;
};

template<class Functor>
class _true_func
{
public:
	const Functor& ftor;
	template<class ...Args>
	bool operator()(Args&... elems) const { ftor(elems...); return true; }
	_true_func(const Functor& ftor) : ftor(ftor) {}
};

template<class Functor>
_true_func<Functor> true_func(Functor&& ftor) {
	return _true_func<Functor>(std::forward<Functor>(ftor));
}

// TODO: source: http://stackoverflow.com/questions/6512019/can-we-get-the-type-of-a-lambda-argument

template<typename F, typename Ret, typename A, typename... Rest>
A helper(Ret (F::*)(A, Rest...));

template<typename F, typename Ret, typename A, typename... Rest>
A helper(Ret (F::*)(A, Rest...) const);

template<typename F>
struct first_argument {
	typedef decltype( helper(&F::operator()) ) type;
};

template<class F>
struct first_argument<_true_func<F>> {
	using type = typename first_argument<typename std::remove_reference<F>::type>::type;
};


/*
template<class Functor1, class Functor2, class Functor3,
	class T1, class T2>() {
		assert(false);
	}

fun_selector<class Functor1, class Functor2, class Functor3>() {
		assert(false);
	}
*/

template<typename F>
using _first_argument = typename first_argument<typename std::remove_reference<F>::type>::type;

// TODO: auto bool deduction by decltype(Functor1)...

template<class Functor1, class Functor2, class Functor3>
fun_selector<Functor1, Functor2, Functor3,
	_first_argument<Functor1>,
	_first_argument<Functor2>>
zipper(Functor1&& ftor1, Functor2&& ftor2, Functor3&& ftor3) {
	return fun_selector<Functor1, Functor2, Functor3,
		_first_argument<Functor1>,
		_first_argument<Functor2>>(
			std::forward<Functor1>(ftor1),
			std::forward<Functor2>(ftor2),
			std::forward<Functor3>(ftor3));
}

template<class Functor1>
fun_selector_symm<Functor1, Functor1, Functor1,
	_first_argument<Functor1>,
	_first_argument<Functor1>
>
zipper(Functor1&& ftor1) {
	return fun_selector_symm<Functor1, Functor1, Functor1,
		_first_argument<Functor1>,
		_first_argument<Functor1>>(
			std::forward<Functor1>(ftor1),
			std::forward<Functor1>(ftor1),
			std::forward<Functor1>(ftor1));
}


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
	return for_each_selection(cont1, cont2, Type<Functor..., decltype(cont1.cbegin()), decltype(cont2.cbegin())>(func...));
}

template<class C1, class C2, class Functor> // TODO: make union a template
bool for_ints_points(const C1& cont1, const C2& cont2, const Functor& func)
{
	return for_each_points<selector_intersection>(cont1, cont2, func);
}

template<class C1, class C2, class Functor> // TODO: make union a template
bool for_union_points(const C1& cont1, const C2& cont2, const Functor& func)
{
	return for_each_points<selector_union>(cont1, cont2, func);
}

#if 0
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
#endif

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
