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

#include "geometry.h"
#include "io.h"

#ifndef GRID_H
#define GRID_H

template<class Traits, class CellTraits>
class _const_cell_itr
{
	using point = _point<Traits>;
	using cell_t = typename CellTraits::cell_t;
	using coord_t = typename Traits::coord_t;
	using dimension = _dimension<Traits>;
protected:
	coord_t linewidth;
	cell_t *ptr, *next_line_end;
	coord_t bw_2;

public:
	//! @param top_left begin of data array
	//! @param dim internal dimension
	//! @param bw border width
	//! @param pos_is_begin whether the itr shall point to begin or end
	_const_cell_itr(cell_t* top_left, dimension dim, coord_t bw,
		bool pos_is_begin = true) :
		linewidth(dim.width()),
		ptr(top_left +
			((pos_is_begin) ? bw * (linewidth+1)
			: dim.area() - bw * (linewidth-1)) ),
		next_line_end(ptr + linewidth - (bw << 1)),
		bw_2(bw << 1)
	{
	}

	_const_cell_itr& operator++()
	{
		// TODO: use a good modulo function here -> no if
		if((++ptr) == next_line_end)
		{
			ptr += bw_2;
			next_line_end += linewidth;
		}
		return *this;
	}

	const cell_t& operator*() const { return *ptr; }
	const cell_t* operator->() const { return ptr; }

	bool operator==(const _const_cell_itr& rhs) const {
		return ptr == rhs.ptr; }
	bool operator!=(const _const_cell_itr& rhs) const {
		return !operator==(rhs); }
};
using const_cell_itr = _const_cell_itr<def_coord_traits, def_cell_const_traits>;

template<class Traits, class CellTraits>
class _cell_itr : public _const_cell_itr<Traits, CellTraits>
{
	using base = _const_cell_itr<Traits, CellTraits>;
	using cell_t = typename CellTraits::cell_t;
public:
	using base::_const_cell_itr;
	cell_t& operator*() { return *base::ptr; }
};
using cell_itr = _cell_itr<def_coord_traits, def_cell_traits>;

template<class Traits>
class grid_alignment_t
{
public:
	using coord_t = typename Traits::coord_t;
	using u_coord_t = typename Traits::u_coord_t;
	using area_t = typename Traits::area_t;
	using point = _point<Traits>;
protected:
	using dimension = _dimension<Traits>;

// TODO: all protected?
	dimension _dim; //! dimension of data, including borders
	u_coord_t bw, bw_2;

	dimension _human_dim() const {
		return dimension(_dim.width() - bw_2, _dim.height() - bw_2);
	}

	/*static area_t storage_area(const dimension& human_dim,
		u_coord_t border_width) {
		const u_coord_t bw_2 = border_width << 1;
		return (human_dim.width() + bw_2) * (human_dim.height() + bw_2);
	}*/
	area_t storage_area() const {
		//const u_coord_t bw_2 = border_width << 1;
		//return (human_dim.width() + bw_2) * (human_dim.height() + bw_2);
		return _dim.area();
	}
public:

	//! returns array index for a human point @a p
/*	int index_internal(const point& p) const {
		return (p.y + bw) * _dim.width() + p.x + bw;
	}*/

	/*
	 * conversion
	 */
	// note: should those not be in the dim class?
	//! returns array index for a human point @a p
	int index_h(const point& p) const {
		const int& w = _dim.width(); // TODO: int
		return ((p.y + bw) * w) + bw + p.x;
	}

	//! returns array index for an internal point @a p
	int index_i(const point& p) const {
		return p.y * _dim.width() + p.x;
	}

	point internal2human(const point& p) const {
		return point(p.x - border_width(), p.y - border_width());
	}

	point human2internal(const point& p) const {
		return point(p.x + border_width(), p.y + border_width());
	}

	//! returns *internal* dimension
	const dimension& internal_dim() const { return _dim; } // TODO: remove this?
	dimension human_dim() const { return _human_dim(); }

	rect human_rect() const {
		return rect(
			point(bw_2, bw_2),
			point(_dim.width() - bw_2 + 1, _dim.height() - bw_2 + 1)
		);
	}

	u_coord_t border_width() const { return bw; }

	u_coord_t dx() const { return _dim.dx() - bw_2; }
	u_coord_t dy() const { return _dim.dy() - bw_2; }
	area_t size() const { return dx() * dy(); }

/*	grid(std::vector<int>& data, const dimension& dim, int border_width) :
		data(data),
		dim(dim),
		border_width(border_width)
	{
	}*/
	_dim_cont<Traits> internal_points() const { return _dim.points(0); }
	_dim_cont<Traits> points() const { return _dim.points(bw); }

	//! returns whether p is in the human rectangle, i.e. (0,0) to ...
	bool contains(const point& p) const {
		return _human_dim().contains(p);
	}

	//! simple constructor: empty grid
	grid_alignment_t(u_coord_t border_width) :
		bw(border_width),
		bw_2(bw << 1)
	{}

	grid_alignment_t(const dimension& dim, u_coord_t border_width) :
		_dim(dim.width() + (border_width << 1),
			dim.height() + (border_width << 1)),
		bw(border_width),
		bw_2(bw << 1) // todo: can we not use the inherited ctor?
	{}

	grid_alignment_t& operator=(const grid_alignment_t& rhs)
	{
	//	assert(bw == rhs.bw);
	//	assert(bw_2 == rhs.bw_2);
		bw = rhs.bw;
		bw_2 = rhs.bw_2;

		_dim = rhs._dim;
		return *this;
	}

	friend serializer& operator<<(serializer& s, const grid_alignment_t& g) {
		return s << g._dim << g.bw; }
	friend deserializer& operator>>(deserializer& s, grid_alignment_t& g) {
		s >> g._dim >> g.bw;
		g.bw_2 = g.bw << 1;
		return s;
	}
};

//! class representing a grid, i.e. an array with h/w + border
//! public functions always take human h/w/dim, if not denoted otherwise
template<class Traits, class CellTraits>
class _grid_t : public grid_alignment_t<Traits>
{
	using cell_t = typename CellTraits::cell_t;
	using base = grid_alignment_t<Traits>;
	using self = _grid_t<Traits, CellTraits>;

	std::vector<cell_t> _data;

	using base::bw;
	using base::bw_2;
	using base::_dim;
public:
	using traits_t = Traits;
	using cell_traits_t = CellTraits;
	using point = typename base::point;
	using area_t = typename base::area_t;
	using u_coord_t = typename base::u_coord_t;
private:
	using rect = _rect<Traits>;
	using dimension = typename base::dimension;

	class line
	{ // TODO: private members?
	public:
		point begin;
		u_coord_t length;
		line(const point& begin, const u_coord_t& length) :
			begin(begin), length(length) {}
	};

	// TODO: alignment class?
	class line_itr : _point_itr<Traits>
	{
		u_coord_t dx;
	public:
		using base = _point_itr<Traits>;
		line_itr(const point& max, const point& min, const point& pos) :
			base(max, min, pos),
			dx(max.x - min.x)
			{}
		line_itr(const point& max, const point& min) :
			line_itr(max, min, min)
			{}

		line_itr& operator++()
		{
			return base::increase_y(), *this;
		}
		line_itr& operator--()
		{
			return base::decrease_y(), *this;
		}

		line_itr& operator=(const line_itr& other)
		{
			return operator=(other), *this;
		}

		line operator*() {
			return line(base::position, dx); }
		const point& operator*() const {
			return line(base::position, dx); }

			//	point* operator->() { return &position; }
	//	const point* operator->() const { return &position; }

		explicit operator bool() const {
			return base::operator bool(); }

		//! @note does currently not compare dx
		bool operator==(const line_itr& rhs) const {
			return base::operator ==(rhs); }

		//! @note does currently not compare dx
		bool operator!=(const line_itr& rhs) const {
			return base::operator !=(rhs); }

		static line_itr from_end(point max, point min) {
			return line_itr(max, min, {min.x, max.y});
		}
	};

	class line_cont_t
	{
		const rect rc;
	public:
		line_cont_t(const rect& rc) : rc(rc) {}
		line_itr begin() const { return line_itr(rc.lr(), rc.ul()); }
		line_itr end() const { return line_itr::from_end(rc.lr(), rc.ul()); }
	};

public:

	line_cont_t lines(const rect& rc) const {
		return line_cont_t(rc); }
	line_cont_t lines() const {
		return lines(base::internal_dim()); }

	void copy_line_to(self& other, const line& m_line, const line& other_line) const {
		const auto& m_begin = _data.begin()
			+ base::index_i(m_line.begin);
		std::copy(m_begin, m_begin + m_line.length,
			other._data.begin()
			+ other.index_i(other_line.begin));
	}

	// TODO: template for grids of other class
	void copy_to_int(self& other, const rect& rc) const {
		if(other.dx() != rc.dx()) throw "copy_to(): Incompatible dx";
		if(other.dy() != rc.dy()) throw "copy_to(): Incompatible dy";
		auto itr = lines(rc).begin();
		auto itr_other = other.lines().begin();
		for(; itr != lines(rc).end(); ++itr, ++itr_other)
		 copy_line_to(other, *itr, *itr_other);
	}

	bool operator<(const _grid_t& rhs) const { return _data < rhs._data; }
	bool operator==(const _grid_t& rhs) const { // TODO! compare dimension, too!
		return _data == rhs._data; }
	bool operator!=(const _grid_t& rhs) const { // TODO! compare dimension, too!
		return ! (*this == rhs); }

	const std::vector<cell_t>& data() const { return _data; }
	std::vector<cell_t>& data() { return _data; } // TODO: remove this soon

/*	point point_of(u_coord_t internal_idx) const {
		const int& w = _dim.width();
		const u_coord_t hum_x = (internal_idx % w) - bw;
		const u_coord_t hum_y = (internal_idx / w) - bw;
		return point(hum_x, hum_y);
	}*/

	//! very slow (but faster than allocating a new array)
	void resize_borders(u_coord_t new_border_width);

	/*//! simple constructor: empty grid
	_grid_t(u_coord_t border_width) :
		bw(border_width),
		bw_2(bw << 1)
	{}*/
	_grid_t(u_coord_t border_width = 0) :
		base(border_width)
	{}

	//! simple constructor: fill grid
	//! dimension is human
	//! @param dim human dimension, border not inclusive
	_grid_t(const dimension& dim, u_coord_t border_width, cell_t fill = 0, cell_t border_fill = std::numeric_limits<cell_t>::min()) :
		//_data(storage_area(dim, border_width), fill),
		/*_dim(dim.width() + (border_width << 1),
			dim.height() + (border_width << 1)),
		bw(border_width),
		bw_2(bw << 1)*/
		base(dim, border_width),
		_data(base::storage_area(), fill) // TODO: segfaults if area = 0
	{
		u_coord_t linewidth = dim.width(),
			storage_lw = linewidth + bw_2;
		area_t top = bw * (storage_lw - 1);
		std::fill_n(_data.begin(), top, border_fill);
		for(std::size_t i = top; i < _data.size() - top; i += storage_lw)
			std::fill_n(_data.begin() + i, bw_2, border_fill);
		std::fill(_data.end() - top, _data.end(), border_fill);
	//	data.assign(data.begin(), data.begin() + top, border_fill);
	}

	//! borderless version
	_grid_t(const dimension& dim)
		: _grid_t(dim, 0, 0) {}

	//! constructor which reads a grid immediatelly
	//! @deprecated use iostream instead
	_grid_t(FILE* fp, u_coord_t border_width) :
		base(border_width)
	{
		read(fp);
	}

	void read(FILE* fp)
	{
		std::vector<int> tmp(_data); // TODO
		read_grid(fp, &tmp, &_dim, bw);
		_data = tmp;
	}

	void write(FILE* fp) const
	{
		std::vector<int> tmp(_data); // TODO
		write_grid(fp, &tmp, &_dim, bw);
	}

	_grid_t& operator=(const _grid_t& rhs)
	{
		base::operator=(rhs);
		_data = rhs._data;
		return *this;
	}

	//!< resets everything except the border to new_value
	//!< @todo not tested yet
	void reset(const cell_t& new_value)
	{
		const u_coord_t dx = _dim.dx();
		area_t max = _dim.area() - (dx + 1) * bw;
		const u_coord_t linewidth = dx - bw_2;

		for(area_t pos = (dx + 1) * bw; pos < max; pos += dx)
		{
			const auto first = _data.begin() + pos;
			std::fill(first, first + linewidth, new_value);
		}
	}

	#if 0
	cell_t direct_access(point p)
	{
		return data[p.y * dim.width() + p.x];
	}

	cell_t operator[](point p)
	{
	/*	const int& w = _dim.width();
		const int& bw = border_width;
		printf("index: %d, ds: %d\n",((p.y + bw) * w) + bw + p.x,(int)data.size());
		return data[((p.y + bw) * w) + bw + p.x];*/
		return direct_access({p.x + bw, p.y+bw});
	}
	#endif

	template<class Cont>
	class multi_cell_ref
	{
		const Cont& cont;
		self& ref;
	public:
		multi_cell_ref(const Cont& cont, self& ref) :
			cont(cont), ref(ref) {}
		multi_cell_ref<Cont>& operator=(const cell_t& c) {
			for(const point& p : cont) { ref[p] = c; }
			return *this;
		}
		const Cont& get_cont() const { return cont; }
		/*bool operator>=(const cell_t& c)
		{
			for(point& p : cont) { ref[p] = c; }
		}*/
	};

	multi_cell_ref<std::vector<point>> operator[](const std::vector<point>& cont)
	{
		return multi_cell_ref<std::vector<point>>(cont, *this);
	}

	multi_cell_ref<std::set<point>> operator[](const std::set<point>& cont)
	{
		return multi_cell_ref<std::set<point>>(cont, *this);
	}

	cell_t& operator[](const point& p)
	{
		return _data[base::index_h(p)];
	}

	const cell_t& operator[](const point& p) const
	{
		return _data[base::index_h(p)];
	}

	const cell_t& at_internal(const point& p) const
	{
		return _data[p.y * _dim.width() + p.x];
	}

	cell_t& at_internal(const point& p)
	{
		return _data[p.y * _dim.width() + p.x];
	}

	const cell_t& at_internal(const area_t& idx) const
	{
		return _data[idx];
	}

	cell_t& at_internal(const area_t& idx)
	{
		return _data[idx];
	}

	using iterator = _cell_itr<Traits, CellTraits>;
	using const_iterator = _const_cell_itr<Traits, cell_traits<const cell_t>>;
	using value_type = cell_t;

	iterator begin() { return iterator(_data.data(), _dim, bw); }
	iterator end() { return iterator(_data.data(), _dim, bw, false); }
	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }
	const_iterator cbegin() const { return const_iterator(_data.data(), _dim, bw); }
	const_iterator cend() const { return const_iterator(_data.data(), _dim, bw, false); }
	// TODO: the last two funcs should have cv qualifier

	//! @param point point in human (TODO?!?!) format
	bool point_is_on_border(const point& p) const {
		return base::human_dim().point_is_on_border(p, 0);
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const _grid_t& g) {
		write_grid(stream, g._data, g._dim, g.bw);
		return stream;
	}

	void print_with_border(std::ostream& stream = std::cout) const
	{
		write_grid(stream, _data, base::internal_dim(), 0);
	}

	//! reads a grid with current border width (0 if default constructed)
	friend std::istream& operator>> (std::istream& stream,
		_grid_t& g) {
		read_grid(stream, g._data, g._dim, g.bw,
			std::numeric_limits<int>::min());
		return stream;
	}

	//! constructor which reads a grid immediatelly
	_grid_t(std::istream& stream, u_coord_t border_width, cell_t border = 0) :
		base(border_width)
	{
		read_grid(stream, _data, _dim, bw, border);
	}

	//! constructor which reads a grid immediatelly
	//! @todo bw 1 as def is deprecated, maybe inherit asm__grid_t in asm_basics.h?
	_grid_t(const char* filename, u_coord_t border_width = 1,
		cell_t border_symbol = std::numeric_limits<cell_t>::min()) :
		base(border_width)
	{
		std::ifstream ifs;
		std::istream* is_ptr;
		if(filename) {
			ifs.open(filename);
			if(!ifs.good())
			 std::cerr << "Error opening infile" << std::endl;
			is_ptr = &ifs;
		}
		else
		 is_ptr = &std::cin;
		read_grid(*is_ptr, _data, _dim, bw, border_symbol);
	}

	friend serializer& operator<<(serializer& s, const _grid_t& g) {
		return s << (const base&)g << g._data; }
	friend deserializer& operator>>(deserializer& s, _grid_t& g) {
		return s >> (base&)g >> g._data; }

	point_itr find_subgrid(const _grid_t& sub, const point_itr& from) const
	{
		// simple algorithm for now...
		dim_cont dc(_dim.height(), _dim.width(), bw);
		//point_itr p = dc.begin(from);
		point_itr p = from;
		bool matches = false;
		// (todo) replace dc.end() by an earlier end
		for(; p != dc.end() && !matches; ++p) // TODO: read "from"
		{
			if(p->x + sub.dx() <= base::dx() && p->y + sub.dy() <= base::dy())
			{
				matches = true;
				point end(p->x, p->y + sub.dy());
				std::cout << "point: " << (*p) << std::endl;
				for(point mp = *p, op  = point::zero();
					(mp < end) && matches;
					mp += point(0,1), op += point(0,1)
					)
				{
					auto first = _data.begin() + base::index(mp);
					auto o_first = sub._data.begin() + sub.index(op);
					matches = matches &&
						std::equal(first, first + sub.dx(), o_first);
				}
			}
		}
		//return std::make_pair(*(--p), matches); // for loop has incremented once to often
		return p;
	}

	//! inserts vertical stripe of length @a ins_len,
	//! starting at x = @a pos. The new stripe is undefined
	void insert_stripe_vert(u_coord_t pos, u_coord_t ins_len);

	//! inserts horizontal stripe of length @a ins_len,
	//! starting at y = @a pos. The new stripe is undefined
	void insert_stripe_hor(u_coord_t pos, u_coord_t ins_len);

};

using grid_t = _grid_t<def_coord_traits, def_cell_traits>;

//! Returns true iff @a idx is on the border for given dimension @a dim
inline bool is_border(const dimension& dim, unsigned int idx) {
	const unsigned int idx_mod_width = idx % dim.width();
	return (idx<dim.width() || idx>= dim.area()-dim.width()
	|| idx_mod_width == 0 || idx_mod_width==dim.width()-1);
}

//! Given an empty vector @a grid, creates grid of dimension @a dim
//! with all cells being @a predefined_value
//! @todo This is highly inefficient. Now in O(n^2), can be done easily in O(n)
//! @todo: remove when grid_t is used everywhere
inline void create_empty_grid(std::vector<int>& grid, const dimension& dim,
	int predefined_value = 0)
{
	grid.resize(dim.area(), predefined_value);
	for(unsigned int i=0; i<dim.area(); i++)
	{
		if(is_border(dim, i))
		 grid[i] = std::numeric_limits<int>::min();
	}
}

inline bool human_idx_on_grid(const int human_grid_size, const int human_idx) {
	return (human_idx >= 0 && human_idx < human_grid_size);
}

template<class T, class CT, class Container, class Functor, class Functor2>
void iterate_grid(_grid_t<T, CT>& grid, const Container& c, int num_states,
	const Functor& ftor,
	const Functor2& contin_ftor = [](){ return true; })
{
	typename CT::cell_t* ref;

	for(auto digit = c.begin();
		contin_ftor() && digit != c.end(); )
	{
		ftor(grid);

		for( digit = c.begin();
			digit != c.end() && (
				ref = &grid[*digit],
				*ref = ((*ref + 1) % num_states)) == 0;
			++digit) // TODO: operator?: ?
		{
		}
	}
}

template<class T, class CT, class Container, class Functor>
bool iterate_grid_bool(_grid_t<T, CT>& grid, const Container& c, int num_states,
	const Functor& ftor)
{
	bool ok = true;
	const auto bool_ftor = [&](_grid_t<T, CT>& grid) {
		ok = ftor(grid);
	};
	const auto cont_ftor = [&]() {
		return ok;
	};
	iterate_grid(grid, c, num_states, bool_ftor, cont_ftor);
	return ok;
}

#endif // GRID_H
