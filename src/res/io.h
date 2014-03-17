/*************************************************************************/
/* sca toolsuite - a toolsuite to simulate sandpile cellular automata.   */
/* Copyright (C) 2011-2012                                               */
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

#ifndef IO_H
#define IO_H

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <cctype> // isdigit()
#include <climits>
#include <boost/graph/adjacency_list.hpp>

//#include "general.h"
struct dimension;

//! Converts internal coordinates into human coordinats.
inline unsigned int human2internal(unsigned int human, int internal_width) {
	const unsigned int human_row = human/(internal_width-2);
	const unsigned int human_col = human%(internal_width-2);
	return ((human_row+1)*internal_width + human_col+1 );
}
//! Converts human coordinates into internal coordinats.
inline unsigned int internal2human(unsigned int internal, int internal_width) {
	const unsigned int internal_row = internal/(internal_width);
	const unsigned int internal_col = internal%(internal_width);
	return ((internal_row-1)*(internal_width-2) + internal_col-1 );
}
#if 0
inline void insert_border(std::vector<int>* grid,
	std::vector<int>::iterator itr,
	int amount)
{
	grid->insert(itr, amount, INT_MIN);
}

enum READ_SIGN
{
	RS_EXPECTED,
	RS_ENDLINE,
	RS_END
};

struct DetailsArrows
{
	typedef int cell_type;

	inline static int arrow_2_int(char read_char)
	{
		switch(read_char)
		{
			case '^': return 0;
			case '>': return 1;
			case 'v': return 2;
			case '<': return 3;
			default: {
				std::string error = "Invalid arrow sign read: ";
				error += read_char;
				throw error;
			}
		}
	}

	inline static int int_2_arrow(const int* int_value)
	{
		static const int arrow_palette[4] = { '^', '>', 'v', '<' }; // TODO: ll, tt ?
		if(*int_value != *int_value % 4) { // TODO: -3
			std::string error = "Integer could not be converted to arrow.";
			throw error;
		}
		else return arrow_palette[*int_value];
	}

	static bool parse_sign(FILE* fp, cell_type& sign)
	{
		char read_char;
		const bool return_value = (fscanf(fp, "%c", &read_char) == 1);
		if(return_value)
		 sign = arrow_2_int(read_char);
		return return_value;
	}

	static void put_sign(FILE* fp, const cell_type& sign) {
		fprintf(fp, "%c", (char)int_2_arrow(&sign));
	}
	static const char* name() { return "rotors"; }
};

struct DetailsDefault
{
	typedef int cell_type;
	static cell_type parse_sign(FILE* fp, cell_type& sign) {
		return (fscanf(fp, "%d", &sign)==1);
	}
	static void put_sign(FILE* fp, const cell_type& sign) {
		fprintf(fp, "%d", sign);
	}
	static const char* name() { return "numbers"; }
};

/**
 * @brief The FileGrid struct
 *
 * A file grid contains equal-length lines looking like "0 11 3 2..."
 * The '\n' can not be a border (since it is not symmetric), so we only
 * have inner borders. Thus, we read: (<number>' ')*'\n'.
 */
template<class Detail>
struct FileGridBase
{
	typedef int cell_type;
	typedef int border_type;
	mutable FILE* fp;
	mutable bool next_is_newline;
	mutable bool just_read_newline = false;
	mutable cell_type next_sign;
	mutable READ_SIGN next_sign_type;
	/*void parse_cell(cell_type& parsed) {

	}*/

	static const char* name() { return Detail::name(); }

	inline READ_SIGN get_next_sign(cell_type &parsed) const
	{
/*		// were '\n' or EOF read the last time?
		if(next_sign_type!=RS_EXPECTED) {
			return next_sign_type;
		}*/

		if(next_is_newline) { // simulate reading newline
			next_is_newline = false;
			return RS_ENDLINE;
		}
puts("...1");
		bool ret_val = Detail::parse_sign(fp, next_sign);
	//	next_is_newline = (fgetc(fp) == '\n');

		char next_sign;
		fread(&next_sign, 1, 1, fp);
		next_is_newline = (next_sign == '\n');

		puts("...2");
		if(next_is_newline)
		{
			puts("...");
			if(just_read_newline) // 2 newlines = EOF
			 return RS_END;
			else
			 just_read_newline = true;
		}

	//	assert(ret_val != 0);
		//return (ret_val==1)?((char_after=='\n')?RS_ENDLINE:RS_EXPECTED):RS_END;


		return (ret_val)?(RS_EXPECTED):RS_END;
	}

	inline READ_SIGN parse_cell(cell_type& parsed, int, int) const
	{
//		puts("READ CELL");
		parsed = next_sign;
//		printf("just read: %d, type %d\n", next_sign, next_sign_type);
		return next_sign_type;
	}

	inline void put_cell(const cell_type& parsed, int, int, READ_SIGN read_sign)
	{
		if(read_sign == RS_ENDLINE)
		 fputc('\n', fp);
		else
		 Detail::put_sign(fp, parsed);
	}

	inline READ_SIGN parse_sep(border_type& parsed, int x, int y, int xoff, int yoff) const
	{
		(void)y; (void)xoff; (void)yoff;
//		puts("READ SEP");

		//return (next_sign_type = get_next_sign(next_sign));
		next_sign_type = get_next_sign(next_sign);
		return next_sign_type;
	}

	inline void put_sep(const border_type& parsed, int x, int y, int xoff, int yoff, READ_SIGN read_sign)
	{
	//	printf("put sep, flag: %d\n", read_sign);
		(void)parsed; (void)x; (void)y; (void)xoff; (void)yoff;
		if(x != -1)
		 fputc((read_sign == RS_ENDLINE)?'\n':' ', fp);
	}

	inline void finalize() {}
	inline void set_cols(int cols) { (void)cols; }
	inline void set_rows(int rows) { (void)rows; }

	FileGridBase(FILE* _fp) : fp(_fp), next_is_newline(false), next_sign_type(RS_EXPECTED) {}
};

typedef FileGridBase<DetailsDefault> FileGrid;
typedef FileGridBase<DetailsArrows> FileArrowGrid;

struct _grid
{
	std::vector<int>& grid;
	dimension& dim;
	const int border_width;
	_grid(std::vector<int>& _grid, dimension& _dim, int _border_width=1)
	 : grid(_grid), dim(_dim), border_width(_border_width)
	{
	}
};

struct _const_grid
{
	const std::vector<int>& grid;
	const dimension& dim;
	const int border_width;
	_const_grid(const std::vector<int>& _grid,
		const dimension& _dim, int _border_width=1)
	 : grid(_grid), dim(_dim), border_width(_border_width)
	{
	}
};

/*
	order of borders being appended:
	55555555
	55555555
	55xxxx22
	22xxxx33
	33444444
	44444444
*/
template<typename grid_type>
struct ArrayGrid
{
	typedef int cell_type;
	typedef int border_type;
	grid_type grid;

	/*void parse_cell(cell_type& parsed) {

	}*/

	inline void set_cols(int cols) {
	//	printf("%d cols => %d\n", cols, cols + (grid.border_width<<1));
		grid.dim.width = cols + (grid.border_width<<1);
	}

	inline void set_rows(int rows) {
	//	printf("%d rows => %d\n", rows, rows + (grid.border_width<<1));
		grid.dim.height = rows + (grid.border_width<<1);
	}


	inline READ_SIGN parse_cell(cell_type& parsed, unsigned int x, unsigned int y) const
	{
# if 0
		if(y >= (grid.dim.height - (grid.border_width<<1))) // can not happen for x
		 return false;

		//int skip = (line_width + (border_width<<1))*border_width - border_width
		int index = grid.dim.width * (y+grid.border_width) + grid.border_width + x;
		//printf("xy: %d, %d, index: %d\n",x, y, index);


		//return grid.grid[index];
		parsed = grid.grid[index];
		return true; // TODO!!!
#endif
	//	printf("x,y,grid.dim.width - (grid.border_width<<1)=%d, %d, %d",
	//		x,y,grid.dim.width - (grid.border_width<<1));
		if(y >= (grid.dim.height - (grid.border_width<<1)))
		 return RS_END;
		else if(x >= (grid.dim.width - (grid.border_width<<1))) // can not happen for x
		 return RS_ENDLINE;
		else {
			int index = grid.dim.width * (y+grid.border_width) + grid.border_width + x;
			parsed = grid.grid[index];
			return RS_EXPECTED;
		}
	}

	inline void put_cell(const cell_type& parsed, int x, int y, READ_SIGN read_sign) {
		// we will insert the beginning border later!
	//	puts("PUT CELL");

/*		int index = grid.dim.width * y + x;
		printf("index: %d\n",index);
		grid.grid[index] = parsed;*/

		// No sense in calculating the index:
		// We have to check for buffer overflows anyways!



		assert(read_sign==RS_EXPECTED);
		 grid.grid.push_back(parsed);
	}

	inline READ_SIGN parse_sep(border_type& parsed, int x, int y,
		int xoff, int yoff) const
	{
		(void)parsed; (void)y; (void)xoff, (void)yoff;
		//printf("grid.dim.width: %d, bw: %d\n",grid.dim.width, (grid.border_width<<1));
	/*	if(y >= (grid.dim.height - (grid.border_width<<1)))
		 return RS_END;
		else if(x >= (grid.dim.width - (grid.border_width<<1))) // can not happen for x
		 return RS_ENDLINE;
		else*/
	//	printf("x:%d, grid.dim.width - (grid.border_width<<1)-1: %d\n",x, grid.dim.width - (grid.border_width<<1)-1);
	//	printf("RESULT: %d\n",x>= (grid.dim.width - (grid.border_width<<1)-1));

		return (x>=(int)(grid.dim.width - (grid.border_width<<1)-1))?RS_ENDLINE:RS_EXPECTED;
	}

	inline void put_sep(const border_type& parsed, int x, int y,
		int xoff, int yoff, READ_SIGN read_sign)
	{
/*		fprintf(stderr,"PUT SEP: %d\n",x==-1 || x == (int)(grid.dim.width - (grid.border_width<<1)));
		(void)parsed; (void)y; (void)xoff; (void)yoff;
		if(x==-1 || x == (int)(grid.dim.width - (grid.border_width<<1)))
		 grid.grid.push_back(INT_MIN);*/
		if(read_sign==RS_ENDLINE){
		/* grid.grid.push_back(INT_MIN);
		 grid.grid.push_back(INT_MIN);*/
			// pushed back right and next left border
			grid.grid.insert(grid.grid.end(), grid.border_width<<1, INT_MIN);
		}
	}

	inline void finalize() {
//		printf("grid.dim.area(): %d, grid.grid.size(): %d\n",grid.dim.area(), grid.grid.size());

		insert_border(&grid.grid, grid.grid.end(),(grid.dim.width-1)*grid.border_width); // bottom
		insert_border(&grid.grid, grid.grid.begin(),(grid.dim.width+1)*grid.border_width); // top

//		insert_border(&grid.grid, grid.grid.end(),(grid.dim.width-1)*grid.border_width); // bottom
//		insert_border(&grid.grid, grid.grid.begin(),(grid.dim.height+1)*grid.border_width); // top

//		for(unsigned int i = 0; i < grid.grid.size(); i++)
//		 fprintf(stderr,"%d\n",grid.grid[i]);

//		printf("grid.dim.area(): %d, grid.grid.size(): %d\n",grid.dim.area(), grid.grid.size());
//		fprintf(stderr,"border width %d\n",grid.border_width);
//		fprintf(stderr,"grid.dim.area(), grid.grid.size(): %d, %d\n",grid.dim.area(), grid.grid.size());
		assert(grid.dim.area() == grid.grid.size());
	}

	ArrayGrid(grid_type _grid) : grid(_grid) {}
};

typedef ArrayGrid<_grid> default_grid;
typedef ArrayGrid<_const_grid> const_default_grid;

template <typename source, typename destination>
struct converter
{
	const source& src;
	destination& dest;

	READ_SIGN convert_cell(int x, int y)
	{
		typename source::cell_type cell_src;
		const READ_SIGN ret_val = src.parse_cell(cell_src, x, y);
		if(ret_val != RS_END)
		 dest.put_cell(cell_src, x, y, ret_val);
		return ret_val;
	}

	READ_SIGN convert_sep(int x, int y, int xoff, int yoff)
	{
		typename source::cell_type cell_src;
		const READ_SIGN ret_val = src.parse_sep(cell_src, x, y, xoff, yoff);
		if(ret_val != RS_END)
		 dest.put_sep(cell_src, x, y, 0, 1, ret_val);
		return ret_val;
	}

	void convert()
	{
		int line_width = -1, x = 0, y = 0; // all excl. border

		// We will insert upper and lower border afterwards, but
		// left and right border are inserted just in time

		//src.initialize();
		dest.set_cols(10000);

	//	bool end_of_input = false;
		READ_SIGN read_sign;
		// do the first row separately
		{
			read_sign = convert_sep(-1, y, 1, 0);
			for(x = 0; read_sign == RS_EXPECTED; ++x)
			{
				read_sign = convert_cell(x, y);
				if(read_sign == RS_EXPECTED) {
					read_sign = convert_sep(x, y, 1, 0);
				//	assert(read_sign==RS_EXPECTED);
				}
			}
		}
	//	fprintf(stderr,"width: %d\n", x-1);

		// we have read 0... width-1 and width ('\n'),
		// now the for loop turned to width+1
		line_width = x;
		dest.set_cols(line_width);

		for(y = 1; read_sign != RS_END; ++y)
		{	
			read_sign = convert_sep(-1, y, 1, 0);
//			printf("read: %d\n",read_sign);
			for(x = 0; read_sign == RS_EXPECTED; ++x)
			{
				read_sign = convert_cell(x, y);
				if(read_sign == RS_EXPECTED) {
					read_sign = convert_sep(x, y, 1, 0);
				//	assert(read_sign==RS_EXPECTED);
				}
			}


		/*	bool inner = convert_sep(x, y, 1, 0);*/



			//sep_dest = sep_src;

			if( read_sign == RS_ENDLINE ) // i.e. a new line
			{
	//			printf("line width is %d, should be %d\n", line_width, x);
			/*	if( y == 0 ) { // first newline => determine line length
					line_width = x;
					dest.set_cols(line_width);
				}
				else*/ {
				if(line_width != x)
					 printf("line width is %d instead of %d in line %d (counted from 0)\n",
						line_width, x, y);
					assert(line_width == x);
				}
				x = 0;
			}

			//dest.put_sep(sep_dest, x, y, 0, 1, inner); // this.left + next.right
		//	convert_sep(x, y, 1, 0);

		}



		dest.set_rows(y-1);
		dest.finalize();


	//	dim->height = line_count + (((int)(border_width))<<1);
	//	dim->width = line_width + (((int)(border_width))<<1);
	}

	converter(const source& _src, destination& _dest) : src(_src), dest(_dest) {}
};
#endif

#if 0
template <typename base>
union chunk
{
	typename base::cell_type cell;
	typename base::border_type border;
	/*template <typename other>
	operator=(line<other>& source_chunk)*/
};

template <typename base>
struct line
{
	std::vector< chunk<base> > data;
	line(int width) {
		data.reserve((width<<1)+1);
	}
	template <typename other>
	operator=(line<other>& source_data) {
		assert(data.size() == source_data.size());
		data[0].border = static_cast< base::border_type >(source_data[0].border);
		for(int i = 0; i < data.size(); i+=2) {
			data[i].cell = static_cast< chunk<base> >(source_data[i].cell);
			data[i+1].border = static_cast< chunk<base> >(source_data[i+1].border);
		}
	}
};

template <typename base>
struct line
{
	std::vector< chunk<base> > data;
	line(int width) {
		data.reserve((width<<1)+1);
	}
	template <typename other>
	operator=(line<other>& source_data) {
		assert(data.size() == source_data.size());
		data[0].border = static_cast< base::border_type >(source_data[0].border);
		for(int i = 0; i < data.size(); i+=2) {
			data[i].cell = static_cast< chunk<base> >(source_data[i].cell);
			data[i+1].border = static_cast< chunk<base> >(source_data[i+1].border);
		}
	}
};

template <typename source, typename destination>
struct converter
{
	const source& src;
	destination& dest;

	void convert()
	{
		int line_width = dest.init_read();

		line<source> line_in(line_width);
		line<dest> line_out(line_width);

		int cells_read = data_read;
		do
		{
			cells_read = src.read_line(line_in);
			if(cells_read)
			{
				line_in = line_out;
				dest.put_line(line_out);
			}
		}
		while(data_read);


#if 0
		int line_width = -1, x = 0, y = 0; // all excl. border




		// We will insert upper and lower border afterwards, but
		// left and right border are inserted just in time

		//src.initialize();
		dest.set_cols(10000);

	//	bool end_of_input = false;
		READ_SIGN read_sign;
		// do the first row separately
		{
			read_sign = convert_sep(-1, y, 1, 0);
			for(x = 0; read_sign == RS_EXPECTED; ++x)
			{
				read_sign = convert_cell(x, y);
				if(read_sign == RS_EXPECTED) {
					read_sign = convert_sep(x, y, 1, 0);
				//	assert(read_sign==RS_EXPECTED);
				}
			}
		}
	//	fprintf(stderr,"width: %d\n", x-1);

		// we have read 0... width-1 and width ('\n'),
		// now the for loop turned to width+1
		line_width = x;
		dest.set_cols(line_width);

		for(y = 1; read_sign != RS_END; ++y)
		{
			read_sign = convert_sep(-1, y, 1, 0);
//			printf("read: %d\n",read_sign);
			for(x = 0; read_sign == RS_EXPECTED; ++x)
			{
				read_sign = convert_cell(x, y);
				if(read_sign == RS_EXPECTED) {
					read_sign = convert_sep(x, y, 1, 0);
				//	assert(read_sign==RS_EXPECTED);
				}
			}


		/*	bool inner = convert_sep(x, y, 1, 0);*/



			//sep_dest = sep_src;

			if( read_sign == RS_ENDLINE ) // i.e. a new line
			{
			/*	if( y == 0 ) { // first newline => determine line length
					line_width = x;
					dest.set_cols(line_width);
				}
				else*/
				 assert(line_width == x);
				x = 0;
			}

			//dest.put_sep(sep_dest, x, y, 0, 1, inner); // this.left + next.right
		//	convert_sep(x, y, 1, 0);

		}



		dest.set_rows(y-1);
		dest.finalize();


	//	dim->height = line_count + (((int)(border_width))<<1);
	//	dim->width = line_width + (((int)(border_width))<<1);
	}
#endif

};
#endif

inline bool is_number_parsable(char sgn)
{
	return isdigit(sgn) || (sgn == '-');
}

inline void read_number(const char*& ptr, int* read_symbol) {
	*read_symbol = atoi(ptr);
	while(is_number_parsable(*++ptr));
}

/**
	Read a grid from a file pointer (given without border)
	@param fp open file, readable
	@param grid pointer to vector, shall be empty and usually not pre-allocated
	@param dim the real dimension of the grid, i.e. including border
	@param SCANFUNC function which converts chars to numbers for internal handling
	@param border how thick the internal border shall be - internal use only
*/
void read_grid(FILE* fp, std::vector<int>* grid, dimension* dim,
	void (*SCANFUNC)(const char*&, int*) = &read_number,
	int border = 1);

inline void read_grid(FILE* fp, std::vector<int>* grid, dimension* dim,
	int border)
{
	read_grid(fp, grid, dim, &read_number, border);
}

#if 0
template<class grid_serializer>
void _read_grid(FILE* fp, std::vector<int>* grid, dimension* dim, int border_width = 1)
{
	grid_serializer ser(grid);
	_read_grid(fp, grid, dim, &ser, border_width);
}

//! @todo: use default templates when c++11 gets default
/*inline void read_grid(FILE* fp, std::vector<int>* grid, dimension* dim, int border = 1)
{
	default_serializer serializer;
	serializer.grid = grid;
	_read_grid<default_serializer>(fp, grid, dim, &serializer, border);
}*/

inline void read_grid(FILE* _fp, std::vector<int>* _vector, dimension* _dim, int _border = 1)
{
	const FileGrid infile(_fp);
	default_grid grid(_grid(*_vector, *_dim, _border));
	converter<FileGrid, default_grid> c(infile, grid);
	c.convert();
}

template<class grid_writer>
inline void read_array(FILE* fp, std::vector<int>* grid, dimension* dim) {
	_read_grid<grid_writer>(fp, grid, dim, 0);
}

/*inline void write_arrow(FILE* fp, int int_to_write) {
	fprintf(fp, "%c", (char)int_2_arrow(&int_to_write));
}*/

inline void write_number(FILE* fp, int int_to_write) {
	fprintf(fp, "%d", int_to_write);
}

/**
	Write a grid to a file pointer (without border)
	If border exists, its values are ignored.
	@param fp open file, writable
	@param grid pointer to vector, shall contain the grid
	@param dim shall contain real dimension of the grid, i.e. including border
	@param PRINTFUNC function which converts numbers to chars for readability
	@param border how thick the internal border shall be - internal use only
*/
template<class grid_serializer>
void _write_grid(FILE* fp, const std::vector<int>* grid, const dimension* dim,
	grid_serializer* serializer, int border)
{
	unsigned int last_symbol = dim->width - 1 - border;

	for(unsigned int y = border; y < dim->height - border; y++)
	{
		for(unsigned int x = border; x < dim->width - border; x++) {
			serializer->write_cell(fp, (*grid)[x + (dim->width)*y]); // TODO: two [] operators
			serializer->write_border(fp, x, y, 1, 0);
			//fputc((x == last_symbol) ? '\n':' ', fp);
		}
		fputc('\n', fp);
		for(unsigned int yoff = 1;
			yoff <= grid_serializer::horizontal_separators_each() - border; yoff++)
		for(unsigned int x = border; x < dim->width - border; x++) {
			serializer->write_border(fp, x, y, 0, yoff); // TODO: two [] operators
			serializer->write_border(fp, x, y, 1, yoff);
			//fputc((x == last_symbol) ? '\n':' ', fp);
		}
		fputc('\n', fp);
	}
}

inline void write_grid(FILE* _fp, const std::vector<int>* _vector, const dimension* _dim,
	int _border_width = 1)
{
	FileGrid outfile(_fp);
	const_default_grid grid(_const_grid(*_vector, *_dim, _border_width));
	converter<const_default_grid, FileGrid> c(grid, outfile);
	c.convert();
}

template<class grid_writer>
inline void write_array(FILE* fp, std::vector<int>* grid, dimension* dim) {
	_write_grid<grid_writer>(fp, grid, dim, 0);
}
#endif

inline void write_number(FILE* fp, int int_to_write) {
	fprintf(fp, "%d", int_to_write);
}

/**
	Write a grid to a file pointer (without border)
	If border exists, its values are ignored.
	@param fp open file, writable
	@param grid pointer to vector, shall contain the grid
	@param dim shall contain real dimension of the grid, i.e. including border
	@param PRINTFUNC function which converts numbers to chars for readability
	@param border how thick the internal border shall be - internal use only
*/
void write_grid(FILE* fp, const std::vector<int>* grid, const dimension* dim,
	void (*PRINTFUNC)(FILE*, int) = &write_number, int border = 1);

inline void write_grid(FILE* fp, const std::vector<int>* grid, const dimension* dim,
	int border) {
	write_grid(fp, grid, dim, &write_number, border);
}


/**
	Executes shell command and pipes output to stdout.
	Useful to get more than one input into your program.
	@param shell_command Command to be executed with sh -c. Can contain any shell syntax.
	@return true iff new process could be started
*/
bool get_input(const char* shell_command);

#include <boost/graph/adjacency_list.hpp>
using namespace boost;
typedef boost::adjacency_list<
		vecS,vecS,directedS, property<vertex_index_t, int>
	> graph_t;
//! to be documented
void create_boost_graph(FILE* fp, graph_t* boost_graph);

//! dumps a boost graph into a trivial graphics file (TGF, see Wikipedia)
void dump_graph_as_tgf(FILE* write_fp, const graph_t* boost_graph);

#endif // IO_H

