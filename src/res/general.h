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

#include <cassert>
#include <exception>
#include <climits>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>

#include "random.h"
#include "io.h"

#ifndef GENERAL_H
#define GENERAL_H
#define SCA_DEBUG

inline void os_sleep(unsigned int seconds) { sleep(seconds); }
inline void os_clear() { system("clear"); }

typedef int coord_t;
typedef unsigned int u_coord_t;
typedef unsigned int area_t;
typedef int cell_t; //!< state of a cell

enum class storage_t
{
	human,
	internal
};

//! Generic structure to store 2D coordinates
struct point
{
	coord_t x, y;
	point(coord_t _x, coord_t _y) : x(_x), y(_y) {}
	point() {}
	void set(int _x, int _y) { x = _x; y = _y; }
	//! one way to compare points: linewise
	bool operator<(const point& rhs) const {
		return (y==rhs.y)?(x<rhs.x):(y<rhs.y);
	}
	inline point operator-(const point& rhs) const {
		return point(x-rhs.x, y-rhs.y);
	}
	inline point operator+(const point& rhs) const {
		return point(x+rhs.x, y+rhs.y);
	}
	inline point& operator-=(const point& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	bool operator==(const point& rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(const point& rhs) const { return ! operator==(rhs); }

	const static point zero;

	friend std::ostream& operator<< (std::ostream& stream,
		const point& p) {
		stream << "(" << p.x << ", " << p.y << ")";
		return stream;
	}
};

struct point_itr
{
	const point min, max;
	point position;
public:
	point_itr(point max, point min, point position) :
		min(min), max(max), position(position)
	{
	}

	point_itr(point max, point min = {0, 0}) :
	//	min(min), max(max), position(min)
		point_itr(max, min, min)
	{
	}

	point& operator*() { return position; }
	const point& operator*() const { return position; }

	point_itr& operator++()
	{
		if(++position.x >= max.x)
		{
			position.x = min.x;
			position.y++;
		}
		return *this;
	}

	bool operator!() const {
		return position.y >= max.y;
	}

	bool operator!=(const point_itr& rhs) const {
		return position != rhs.position; }

	static point_itr from_end(point max, point min = {0,0}) {
		return point_itr(max, min, {min.x, max.y});
	}
};

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

struct dimension_container
{
	// TODO: dimenion as member?
	const unsigned h, w, bw;
	dimension_container(unsigned h,
		unsigned w,
		unsigned bw)
		: h(h), w(w), bw(bw)
	{}

	point_itr begin() const { return point_itr(
		{(coord_t)(w-(bw<<1)), (coord_t)(h-(bw<<1))}
		);
	}
	point_itr end() const {
		return point_itr::from_end(
			{(coord_t)(w-(bw<<1)), (coord_t)(h-(bw<<1))}
			);
	}
};

//! Generic structure for a 2D rectangle dimension.
struct dimension
{
	u_coord_t height;
	u_coord_t width;
	inline unsigned int area() const { return height*width; }
	inline area_t area_without_border(u_coord_t border_size = 1) const {
		const u_coord_t bs_2 = border_size << 1;
		assert(height >= bs_2); assert(width >= bs_2);
		return (height-bs_2)*(width-bs_2);
	}
	inline bool operator==(const dimension& other) const {
		return height == other.height && width == other.width;
	}
	inline bool operator!=(const dimension& other) const {
		return !(operator ==(other));
	}
	int coords_to_id(int x, int y) const { return y * width + x; }
	int coords_to_id(const point& p) const {
		return coords_to_id(p.x, p.y);
	}
	void id_to_coords(int id, int* x, int* y) const {
		*y = id / width;
		*x = id - (*y) * width;
	}
	point id_to_coords(int id) const {
		const int y = id / width;
		return point(id - y * width, y);
	}

	bool point_is_on_border(const point& p,
		const u_coord_t border_size) const {
		return p.x < (coord_t) border_size
		|| p.x >= (coord_t) width - (coord_t) border_size
		|| p.y < (coord_t) border_size
		|| p.y >= (coord_t) height - (coord_t) border_size;
	}

	dimension_container points(unsigned border_width) const {
		return dimension_container(height, width, border_width); }
};

// TODO: protected members
struct rect
{
	point ul, lr;
	rect(const point& ul, const point& lr) : ul(ul), lr(lr) {}
	//! constructs the rect from the inner part of a dim,
	//! i.e. dim - border
	rect(const dimension& d, const coord_t border_size = 0) :
		ul(0, 0),
		lr(d.width - (border_size << 1),
			d.height - (border_size << 1)) {}

	inline area_t area() const { return (lr.x - ul.x) * (lr.y - ul.y); }
	bool is_inside(const point& p) const {
		return p.x < lr.x && p.y < lr.y && ul.x <= p.x && ul.y <= p.y; }

	point_itr begin() const { return point_itr(lr, ul); }
	point_itr end() const { return point_itr::from_end(lr, ul); }

	friend std::ostream& operator<< (std::ostream& stream,
		const rect& r) {
		stream << "rect (" << r.ul << ", " << r.lr << ")";
		return stream;
	}
};

// TODO: inherit from bounding box
class bounding_box
{
	point _ul, _lr;
public:
	bounding_box() : _ul(1,1), _lr(0,0) {}
	void add_point(const point& p)
	{
		if(_lr.x < _ul.x) // no point...
			_lr = _ul = p;
		else
		{
			if(p.x < _ul.x)
			 _ul.x = p.x;
			else if(p.x > _lr.x)
			 _lr.x = p.x;
			if(p.y < _ul.y)
			 _ul.y = p.y;
			else if(p.y > _lr.y)
			 _lr.y = p.y;
		}
	}
	// TODO: for those 3 cases: cover lr > ul
	point ul() const { return _ul; }
	point lr() const { return _lr; }
	dimension dim() const {
		return dimension { (unsigned)_lr.y - _ul.y,
			(unsigned)_lr.x - _ul.x };
	}
};

class cell_itr
{
	coord_t linewidth;
	cell_t *ptr, *next_line_end;
	coord_t bw_2;

public:
	cell_itr(cell_t* top_left, dimension dim, coord_t bw,
		bool pos_is_begin = true) :
		linewidth(dim.width),
		ptr(top_left +
			((pos_is_begin) ? bw * (linewidth+1)
			: dim.area() - bw * (linewidth-1)) ),
		next_line_end(ptr + linewidth - (bw << 1)),
		bw_2(bw << 1)
	{
	}

	cell_itr& operator++()
	{
		if((++ptr) == next_line_end)
		{
			ptr += bw_2;
			next_line_end += linewidth;
		}
		return *this;
	}

	const cell_t& operator*() const { return *ptr; }
	cell_t& operator*() { return *ptr; }

	bool operator==(const cell_itr& rhs) const { return ptr == rhs.ptr; }
	bool operator!=(const cell_itr& rhs) const { return !operator==(rhs); }
};

struct grid_t // TODO: class
{
	std::vector<cell_t> data;
	dimension _dim; //! dimension of data, including borders
	const u_coord_t border_width;

	//! returns array index for a human point @a p
	int index(const point& p) const {
		const int& w = _dim.width;
		const int& bw = border_width;
		return ((p.y + bw) * w) + bw + p.x;
	}

	dimension human_dim() const {
		u_coord_t bw_2 = border_width << 1;
		return dimension { _dim.height - bw_2, _dim.width - bw_2 };
	}

	static area_t internal_area(const dimension& human_dim,
		u_coord_t border_width) {
		u_coord_t bw_2 = border_width << 1;
		return (human_dim.width + bw_2) * (human_dim.height + bw_2);
	}
public:
	//! returns *internal* dimension
	const dimension& dim() const { return _dim; } // TODO: remove this?

	const area_t size() const { return
		_dim.area_without_border(border_width); }

/*	grid(std::vector<int>& data, const dimension& dim, int border_width) :
		data(data),
		dim(dim),
		border_width(border_width)
	{
	}*/

	dimension_container points() const { return _dim.points(border_width); }

	//! simple constructor: empty grid
	grid_t(u_coord_t border_width) :
		border_width(border_width)
	{}

	//! simple constructor: fill grid
	//! dimension is human
	grid_t(u_coord_t border_width,
		const dimension& dim, cell_t fill = 0) :
		data(internal_area(dim, border_width), fill),
		_dim(dim),
		border_width(border_width)
	{}

	//! constructor which reads a grid immediatelly
	grid_t(FILE* fp, u_coord_t border_width) :
		border_width(border_width)
	{
		read(fp);
	}

	void read(FILE* fp)
	{
		read_grid(fp, &data, &_dim, border_width);
	}

	void write(FILE* fp) const
	{
		write_grid(fp, &data, &_dim, border_width);
	}

	grid_t& operator=(const grid_t& rhs)
	{
		assert(border_width == rhs.border_width);
		data = rhs.data;
		_dim = rhs._dim;
		return *this;
	}

	#if 0
	cell_t direct_access(point p)
	{
		return data[p.y * dim.width + p.x];
	}

	cell_t operator[](point p)
	{
	/*	const int& w = _dim.width;
		const int& bw = border_width;
		printf("index: %d, ds: %d\n",((p.y + bw) * w) + bw + p.x,(int)data.size());
		return data[((p.y + bw) * w) + bw + p.x];*/
		return direct_access({p.x + bw, p.y+bw});
	}
	#endif

	cell_t& operator[](point p)
	{
		return data[index(p)];
	}

	const cell_t& operator[](point p) const
	{
		return data[index(p)];
	}

	cell_itr begin() { return cell_itr(data.data(), _dim, border_width); }
	cell_itr end() { return cell_itr(data.data(), _dim, border_width, false); }

	bool point_is_on_border(const point& p) const {
		return human_dim().point_is_on_border(p, 0);
	}

//	friend std::ostream& operator<< (std::ostream& stream,
//		const configuration& c);
};

//! Returns true iff @a idx is on the border for given dimension @a dim
inline bool is_border(const dimension* dim, unsigned int idx) {
	const unsigned int idx_mod_width = idx % dim->width;
	return (idx<dim->width || idx>= dim->area()-dim->width
	|| idx_mod_width == 0 || idx_mod_width==dim->width-1);
}

//! Given an empty vector @a grid, creates grid of dimension @a dim
//! with all cells being @a predefined_value
//! @todo This is highly inefficient. Now in O(n^2), can be done easily in O(n)
inline void create_empty_grid(std::vector<int>* grid, const dimension* dim,
	int predefined_value = 0)
{
	grid->resize(dim->area(), predefined_value);
	for(unsigned int i=0; i<dim->area(); i++)
	{
		if(is_border(dim, i))
		 (*grid)[i] = INT_MIN;
	}
}

inline bool human_idx_on_grid(const int human_grid_size, const int human_idx) {
	return (human_idx >= 0 && human_idx < human_grid_size);
}

//! Generic structure to store help commandline information in and print it
struct HelpStruct
{
	const char* description;
	const char* syntax;
	const char* input;
	const char* output;
	typedef std::vector<std::pair<const char*, const char*> > param_vec;
	param_vec params;
	const char* return_value;
	inline void print_usage () const {
		if(syntax) fprintf(stderr, "Usage: %s\n", syntax);
	}
	void print_help() const
	{
		fprintf(stderr, "%s\n", description);

		if(input||output) {
			fputs("\n",stderr);
			if(input) fprintf(stderr, "Input (stdin): %s\n", input);
			if(output) fprintf(stderr, "Output (stdout): %s\n", output);
		}

		fputs("\n",stderr);
		print_usage();
		if(params.size()) fprintf(stderr, "Arguments:\n");
		for(param_vec::const_iterator itr = params.begin();
			itr != params.end(); itr++)
		 fprintf(stderr, " * %s: %s\n", itr->first, itr->second);

		if(return_value) fprintf(stderr, "Return value: %s\n", return_value);
		fputs("\nSyntax rules are explained in the file DOCUMENTATION.\n", stderr);
	}
	inline void add_param(const char* name, const char* descr) {
		params.push_back(std::make_pair(name,descr));
	}

	HelpStruct()
		: description(NULL), syntax(NULL), // non-NULL params
		input(NULL), output(NULL), return_value(NULL) {}
};

//! Helper struct for all programs for help displaying and error handling.
class Program
{
protected:
	const bool env_debug;
	int argc;
	char** argv;
private:
	const HelpStruct* help;

	inline void print_termination_string(void) const {
		fprintf(stderr, "%s: Terminating on fatal error.\n", argv[0]);
	}
	inline void internal_exit(void) const { ::exit(EXIT_FAILURE); }

public:
	//! Routine for inherited classes to call to start main().
	int run(int _argc, char** _argv, const HelpStruct* _help)
	{
		argc = _argc; argv = _argv; help = _help;

		assert(help->description && help->syntax);
		if(argc > 1 && (!strcmp(argv[1],"--help")||!strcmp(argv[1],"-help")))
		{
			help->print_help();
			::exit(0);
		}

		int return_value = 0;
		try {
			return_value = main();
		} catch(const char* str) {
			exit(str);
		} catch(const std::string& str) {
			exit(str.c_str());
		}/* catch(std::exception e) {
			exit(e.what());
		} catch(...) {
			exit("Unknown error caught. This should never happen.");
		}*/
		return return_value;
	}
	static inline int safe_atoi(const char* str) { return str?atoi(str):0; }
	Program() : env_debug(safe_atoi(getenv("SCA_DEBUG"))) {}
	virtual ~Program() {}

protected:
	//! Main routine for inherited classes.
	virtual int main() = 0;

	//! Exit program with message @a str
	inline void exit(const char* str) const
	{
		print_termination_string();
		fprintf(stderr, "%s\n", str);
		internal_exit();
	}

	//! Exit program with format string @a format
	inline void exitf(const char* format, ...) const
	{
		print_termination_string();
		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		internal_exit();
	}

	//! Prints str if shell sets SCA_DEBUG and macro SCA_DEBUG is defined.
	//! Programmers can put these everywhere without runtime worries.
	inline void debug(const char* str) const {
#ifdef SCA_DEBUG
		if(env_debug) fputs(str, stderr);
#endif
	}

	//! Prints format if shell sets SCA_DEBUG and macro SCA_DEBUG is
	//! defined.
	//! Programmers can put these everywhere without runtime worries
	inline void debugf(const char* format, ...) const
	{
#ifdef SCA_DEBUG
		if(env_debug)
		{
			va_list args;
			va_start(args, format);
			vfprintf(stderr, format, args);
			va_end(args);
		}
#endif
	}

	//! Exit program with usage
	inline void exit_usage() const
	{
		help->print_usage();
		fprintf(stderr, " (--help for more information)\n");
		internal_exit();
	}

	//! Exit program if @a assertion is false, printing usage
	inline void assert_usage(bool assertion) const { if(!assertion) exit_usage(); }
};

#endif // GENERAL_H
