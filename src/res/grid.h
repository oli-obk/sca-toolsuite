#include "geometry.h"
#include "io.h"

// TODO: license

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
protected:
	using coord_t = typename Traits::coord_t;
	using u_coord_t = typename Traits::u_coord_t;
	using area_t = typename Traits::area_t;
	using point = _point<Traits>;
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
	area_t storage_area() {
		//const u_coord_t bw_2 = border_width << 1;
		//return (human_dim.width() + bw_2) * (human_dim.height() + bw_2);
		return _dim.area();
	}
public:

	//! returns array index for a human point @a p
	int index_internal(const point& p) const {
		return (p.y + bw) * _dim.width() + p.x + bw;
	}

	/*
	 * conversion
	 */
	// note: should those not be in the dim class?
	//! returns array index for a human point @a p
	int index(const point& p) const {
		const int& w = _dim.width(); // TODO: int
		return ((p.y + bw) * w) + bw + p.x;
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

	rect human_rect() const { return human_dim() + point(bw, bw); }

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

	_dim_cont<Traits> points() const { return _dim.points(bw); }

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
};

//! class representing a grid, i.e. an array with h/w + border
//! public functions always take human h/w/dim, if not denoted otherwise
template<class Traits, class CellTraits>
class _grid_t : public grid_alignment_t<Traits>
{
	using cell_t = typename CellTraits::cell_t;
	using base = grid_alignment_t<Traits>;

	std::vector<cell_t> _data;

	using base::bw;
	using base::bw_2;
	using base::_dim;
	using area_t = typename base::area_t;
	using u_coord_t = typename base::u_coord_t;
	using dimension = typename base::dimension;

public:
	bool operator<(const _grid_t& rhs) const { return _data < rhs._data; }
	bool operator==(const _grid_t& rhs) const {
		return _data == rhs._data; }

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

	template<class Point>
	cell_t& operator[](Point p)
	{
		return _data[base::index(p)];
	}

	template<class Point>
	const cell_t& operator[](Point p) const
	{
		return _data[base::index(p)];
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

	//! constructor which reads a grid immediatelly
	_grid_t(std::istream& stream, u_coord_t border_width) :
		base(border_width)
	{
		read_grid(stream, _data, _dim, bw);
	}

	//! constructor which reads a grid immediatelly
	//! @todo bw 1 as def is deprecated, maybe inherit asm__grid_t in asm_basics.h?
	_grid_t(const char* filename, u_coord_t border_width = 1) :
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
		read_grid(*is_ptr, _data, _dim, bw);
	}

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

#endif // GRID_H
