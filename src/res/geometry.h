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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cassert>
#include <vector>
#include <set>
#include <fstream>
#include <type_traits>
#include <limits>
#include <iostream>
#include <cstdint>

template<class T>
struct area_class { using type = unsigned int; };
template<>
struct area_class<char> { using type = unsigned short; };
template<>
struct area_class<int8_t> { using type = unsigned short; };
template<>
struct area_class<uint8_t> { using type = unsigned short; };

//! @arg Coord The type for using coords. signed or unsigned
template<class Coord, class Area = typename area_class<Coord>::type> // TODO: def for area
struct coord_traits
{
	using coord_t = Coord;
	using u_coord_t = typename std::make_unsigned<coord_t>::type; // TODO...
	using area_t = typename std::make_unsigned<Area>::type;
};

template<class Cell>
struct cell_traits
{
	using cell_t = Cell;
};

using def_coord_traits = coord_traits<int>;
using def_cell_traits = cell_traits<int>;
using def_cell_const_traits = cell_traits<const int>;

enum class storage_t
{
	human,
	internal
};

//! Generic structure to store 2D coordinates
//! Comparison is defined as linewise.
template<class Traits>
struct _point
{
	using coord_t = typename Traits::coord_t;
	coord_t x, y;
	constexpr _point(coord_t _x, coord_t _y) noexcept(coord_t(coord_t())) :
		x(_x), y(_y) {}
	_point() {}
	constexpr _point(const _point& other) : x(other.x), y(other.y) {}
	void set(int _x, int _y) { x = _x; y = _y; }
	bool operator<(const _point& rhs) const {
		return (y==rhs.y)?(x<rhs.x):(y<rhs.y);
	}
	bool operator>(const _point& rhs) const {
		return (y==rhs.y)?(x>rhs.x):(y>rhs.y);
	}
	bool operator>=(const _point& rhs) const {
		return !operator<(rhs);
	}
	bool operator<=(const _point& rhs) const {
		return !operator>(rhs);
	}
	inline _point operator-(const _point& rhs) const {
		return _point(x-rhs.x, y-rhs.y);
	}
	inline _point operator+(const _point& rhs) const {
		return _point(x+rhs.x, y+rhs.y);
	}
	inline _point operator+=(const _point& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	inline _point& operator-=(const _point& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	bool operator==(const _point& rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(const _point& rhs) const { return ! operator==(rhs); }

	static constexpr _point zero() { return {0, 0}; }
	static constexpr _point max() {
		return {std::numeric_limits<coord_t>::max(),
			std::numeric_limits<coord_t>::max()}; }
	static constexpr _point min() {
		return {std::numeric_limits<coord_t>::min(),
			std::numeric_limits<coord_t>::min()}; }

	friend std::ostream& operator<< (std::ostream& stream,
		const _point& p) {
		return stream << "(" << (int)p.x << ", " << (int)p.y << ")";
	}
};
using point = _point<def_coord_traits>;

template<class Traits>
struct _point_itr
{
	using coord_t = typename Traits::coord_t;
	using point = _point<Traits>;

	//! max should have x and y behind the last column and row
	//! like STL iterators
	point min, max;
	point position;
public:
	_point_itr& operator=(const _point_itr& other)
	{
		min = other.min;
		max = other.max;
		position = other.position;
		return *this;
	}

	_point_itr(point max, point min, point position) :
		min(min), max(max), position(position)
	{
	}

	_point_itr(point max, point min = {0, 0}) :
		_point_itr(max, min, min)
	{
	}

	_point_itr() {}

	point& operator*() { return position; }
	const point& operator*() const { return position; }
	point* operator->() { return &position; }
	const point* operator->() const { return &position; }

	//! increases the y coordinate of the point
	//! @note There is currently not check for overstepping @a end()
	_point_itr& increase_y()
	{
		return ++position.y, *this;
	}

	//! decreases the y coordinate of the point
	//! @note There is currently not check for overstepping @a begin()
	_point_itr& decrease_y()
	{
		return --position.y, *this;
	}

	_point_itr& operator++()
	{
		if(++position.x >= max.x)
		{
			position.x = min.x;
			++position.y;
		}
		return *this;
	}

/*	_point_itr& operator+=(const coord_t& x_offset)
	{
	}*/

	_point_itr& operator--()
	{
		if(--position.x < min.x)
		{
			position.x = max.x - 1;
			--position.y;
		}
		return *this;
	}

	// TODO: operator +, operator +=

	bool operator!() const {
		return position.y >= max.y;
	}

	explicit operator bool() const { return position.y < max.y; }

	bool operator==(const _point_itr& rhs) const {
		return position == rhs.position; }

	bool operator!=(const _point_itr& rhs) const {
		return !operator==(rhs); }

	static _point_itr from_end(point max, point min = {0,0}) {
		return _point_itr(max, min, {min.x, max.y});
	}

	using value_type = point;
};
using point_itr = _point_itr<def_coord_traits>;

// TODO: which ctors/functions can be made constexpr? everywhere...

//! Generic structure to store a 2D matrix
class matrix
{
	int data[2][2];
public:
	point operator*(const point& rhs) const
	{
		return point(data[0][0]*rhs.x + data[0][1]*rhs.y,
			data[1][0]*rhs.x + data[1][1]*rhs.y);
	}
	matrix operator*(const matrix& rhs) const
	{
		return matrix(data[0][0]*rhs.data[0][0] + data[0][1]*rhs.data[1][0],
			data[0][0]*rhs.data[0][1] + data[0][1]*rhs.data[1][1],
			data[1][0]*rhs.data[0][0] + data[1][1]*rhs.data[1][0],
			data[1][0]*rhs.data[0][1] + data[1][1]*rhs.data[1][1]
			);
	}
	matrix operator^(unsigned n) const {
		matrix result(1,0,0,1);
		for(unsigned i = 0; i < n; ++i)
		 result = result * (*this);
		return result;
	}
	matrix(int a, int b, int c, int d) : data {{a,b},{c,d}} {}
	const static matrix id;
};

//! a container which starts counting from zero, and which has a border width
template<class Traits>
struct _dim_cont
{
	using point = _point<Traits>;
	using const_iterator = _point_itr<Traits>;
	using iterator = const_iterator;
	using value_type = point;
	using coord_t = typename Traits::coord_t;
	using area_t = typename Traits::area_t;

	// TODO: dimension as member?
	const unsigned h, w, bw;
	const iterator _begin, _end;
	_dim_cont(unsigned h,
		unsigned w,
		unsigned bw) :
		h(h), w(w), bw(bw),
		_begin({(coord_t)(w-(bw<<1)), (coord_t)(h-(bw<<1))}),
		_end(iterator::from_end(
			{(coord_t)(w-(bw<<1)), (coord_t)(h-(bw<<1))}
			)) // TODO: use _begin for this?
	{}

	iterator begin(/*const point& pos = point::zero()*/) const noexcept
	 // TODO: argument deprecated?
	{
		return _begin;
	}

	iterator end() const noexcept {
		return _end;
	}

	iterator cbegin(const point& pos = point::zero()) const { return begin(pos); }
	iterator cend() const noexcept { return end(); }
	area_t size() const noexcept { return h * w; }
	bool empty() const noexcept { return (h | w) == 0; }

	friend std::ostream& operator<< (std::ostream& stream,
		const _dim_cont& d) {
		return stream << "dim_cont (" << d.h << " x " << d.w << ")";
	}
};

using dim_cont = _dim_cont<def_coord_traits>;

// TODO: protected members
template<class Traits>
class _rect_storage_default
{
	using point = _point<Traits>;
protected:
	point _ul, _lr;
	_rect_storage_default(const point& _ul, const point& lr) :
		_ul(_ul), _lr(lr) {}
public:
	const point& ul() const { return _ul; }
};

template<class Traits>
class _rect_storage_origin
{
	using point = _point<Traits>;
protected:
	//static constexpr point ul = {0, 0};
	point _lr;
	_rect_storage_origin(const point& , const point& lr) : _lr(lr) {}
	_rect_storage_origin(){} // TODO...
	// TODO: remove first param?
public:
	static constexpr point ul() { return {0, 0}; }
};

// TODO: storage should be a template: template<class T> class storage
template<class Traits, class storage = _rect_storage_default<Traits>>
struct _rect : public storage
{
	using point = _point<Traits>;
	using coord_t = typename Traits::coord_t;
	using area_t = typename Traits::area_t;
	using u_coord_t = typename Traits::u_coord_t;
protected:
	_rect() {} // TODO... this should never be allowed
	using s = storage;
public:
	//! @param lr point *past* lowest right point
	_rect(const point& ul, const point& lr) : storage(ul, lr) {}
	//! constructs the rect from the inner part of a dim,
	//! i.e. dim - border
	// TODO: deprecate this? dangerous...
	_rect(const _rect<Traits, _rect_storage_origin<Traits>>& d, const coord_t border_size = 0) :
		storage({0, 0},
			{(coord_t)(d.width() - (border_size << 1)), // TODO: ?????
			(coord_t)(d.height() - (border_size << 1))}) {}

	const point& lr() const { return s::_lr; }

	inline area_t area() const { return (s::_lr.x - s::ul().x) * (s::_lr.y - s::ul().y); }
	bool is_inside(const point& p) const {
		return p.x < s::_lr.x && p.y < s::_lr.y && s::ul().x <= p.x && s::ul().y <= p.y; }

	using const_iterator = _point_itr<Traits>;
	using iterator = const_iterator;
	iterator begin() const { return iterator(s::_lr, s::ul()); }
	iterator end() const { return iterator::from_end(s::_lr, s::ul()); }
	iterator cbegin() const { return begin(); }
	iterator cend() const { return end(); }
	inline area_t size() const { return area(); }

	// TODO: this makes only sense for dim?
	_dim_cont<Traits> points(u_coord_t border_width) const {
		return _dim_cont<Traits>(height(), width(), border_width); }

	friend std::ostream& operator<< (std::ostream& stream,
		const _rect& r) {
		return stream << "rect (" << r.s::ul() << ", " << r.s::_lr << ")";
	}

	inline area_t area_without_border(u_coord_t border_size = 1) const {
		const u_coord_t bs_2 = border_size << 1;
		assert(height() >= bs_2); assert(width() >= bs_2);
		return (height()-bs_2)*(width()-bs_2);
	}

	_rect<Traits> operator+(const point& rhs) const {
		return _rect<Traits, _rect_storage_default<Traits>>(s::ul() + rhs, s::_lr + rhs);
	}

	// TODO: should we also allow borders in all our computations?
	bool point_is_on_border(const point& p,
		const u_coord_t border_size) const {
		return p.x < s::ul().x + (coord_t) border_size
		|| p.x >= s::_lr.x - (coord_t) border_size
		|| p.y < s::ul().y + (coord_t) border_size
		|| p.y >= s::_lr.y - (coord_t) border_size;
	}

	inline u_coord_t dx() const { return s::_lr.x - s::ul().x; }
	inline u_coord_t dy() const { return s::_lr.y - s::ul().y; }

	inline u_coord_t width() const { return dx(); }
	inline u_coord_t height() const { return dy(); }

	inline bool operator==(const _rect& other) const {
		return s::_lr == other.s::_lr && s::ul() == other.s::ul();
	}
	inline bool operator!=(const _rect& other) const {
		return !(operator ==(other));
	}

	bool point_is_contained(const point& p) const {
		return p.x >= s::ul().x && p.x < s::_lr.x
			&& p.y >= s::ul().y && p.y < s::_lr.y;
	}
	inline bool fits_points(const std::set<point>& points) const
	{
		if(points.size() == area())
		{
			const point& offset = *points.begin();
			(void)offset;
			for(const point& p : points)
			{
				std::cout << p << std::endl;
				if(!point_is_contained(p))
				 return false;
			}
		}
		else
		 return false;
		return true;
	}
};
using rect = _rect<def_coord_traits, _rect_storage_default<def_coord_traits>>;

template<class Traits>
struct _dimension : public _rect<Traits, _rect_storage_origin<Traits>>
{
	using coord_t = typename Traits::coord_t;
	using u_coord_t = typename Traits::u_coord_t;
	using point = _point<Traits>;
	_dimension(u_coord_t width, u_coord_t height) :
		_rect<Traits, _rect_storage_origin<Traits>>({0,0}, {(coord_t)width, (coord_t)height}) {}
	_dimension() {}
};
using dimension = _dimension<def_coord_traits>;

// TODO: inherit from bounding box
template<class Traits>
class _bounding_box
{
	using point = _point<Traits>;
	using dimension = _dimension<Traits>;
	using u_coord_t = typename Traits::u_coord_t;
	point _ul, _lr;
public:
	constexpr _bounding_box() : _ul(point::zero()), _lr(point::zero()) {}

	void add_point(const point& p)
	{
		if(_lr.x == _ul.x) { // bb was empty
			_ul = p;
			_lr.x = p.x + 1;
			_lr.y = p.y + 1;
		}
		else
		{
			if(p.x < _ul.x)
			 _ul.x = p.x;
			else if(p.x >= _lr.x)
			 _lr.x = p.x + 1;
			if(p.y < _ul.y)
			 _ul.y = p.y;
			else if(p.y >= _lr.y)
			 _lr.y = p.y + 1;
		}
	}
	// TODO: for those 3 cases: cover lr > ul
	u_coord_t y_size() const { return _lr.y - _ul.y; }
	u_coord_t x_size() const { return _lr.x - _ul.x; }
	point ul() const { return _ul; }
	point lr() const { return _lr; }
	dimension dim() const {
		return dimension { (u_coord_t) y_size(),
			(u_coord_t) x_size() };
	}
	u_coord_t coords_to_id(const point& p) const {
		return (p.y - _ul.y) * y_size() + (p.x - _ul.x);
	}
};
using bounding_box = _bounding_box<def_coord_traits>;

#endif // GEOMETRY_H
