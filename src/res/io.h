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

#ifndef IO_H
#define IO_H

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <cctype> // isdigit()
#include <climits>
#include <unistd.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

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

class number_grid
{
	inline static bool is_number_parsable(char sgn)
	{
		return isdigit(sgn) || (sgn == '-');
	}
public:
	inline static void write(char*& ptr, int int_to_write) {
		ptr += sprintf(ptr, "%d", int_to_write);
	}

	inline static void read(const char*& ptr, int* read_symbol) {
		*read_symbol = atoi(ptr);
		while(is_number_parsable(*++ptr));
	}

	const char* name() { return "numbers"; }
};

struct arrow_grid
{
	using cell_type = int;
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

	inline static char int_2_arrow(const int* int_value)
	{
		static const int arrow_palette[4] = { '^', '>', 'v', '<' }; // TODO: ll, tt ?
		if(*int_value != *int_value % 4) { // TODO: -3
			std::string error = "Integer could not be converted to arrow.";
			throw error;
		}
		else return arrow_palette[*int_value];
	}

/*	static bool parse_sign(FILE* fp, cell_type& sign)
	{
		char read_char;
		const bool return_value = (fscanf(fp, "%c", &read_char) == 1);
		if(return_value)
		 sign = arrow_2_int(read_char);
		return return_value;
	}

	static void put_sign(FILE* fp, const cell_type& sign) {
		fprintf(fp, "%c", (char)int_2_arrow(&sign));
	}*/

	inline static void write(char*& ptr, int int_to_write) {
//		ptr += sprintf(ptr, "%d", int_to_write);
		*(ptr++) = int_2_arrow(&int_to_write);
	}

	inline static void read(const char*& ptr, int* read_symbol) {
		//*read_symbol = atoi(ptr);
		//while(is_number_parsable(*++ptr));
		*read_symbol = arrow_2_int(*(ptr++));
	}


	const char* name() { return "rotors"; }
};

inline bool _is_number_parsable(char sgn) {
	return isdigit(sgn) || (sgn == '-');
}

inline static void write_number(char*& ptr, int int_to_write) {
	ptr += sprintf(ptr, "%d", int_to_write);
}

inline static void read_number(const char*& ptr, int* read_symbol) {
	*read_symbol = atoi(ptr);
	while(_is_number_parsable(*++ptr));
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

void read_grid(std::istream& is, std::vector<int>& grid, dimension& dim,
	void (*SCANFUNC)(const char *&, int *), int border = -1);

inline void read_grid(FILE* fp, std::vector<int>* grid, dimension* dim,
	int border)
{
	read_grid(fp, grid, dim, &read_number, border);
}

/*
void read_grid(FILE* fp, std::vector<int>* grid, dimension* dim,
	void (*SCANFUNC)(const char*&, int*) = &read_number,
	int border = 1);

void read_grid(std::istream& is, std::vector<int>& grid, dimension& dim,
	void (*SCANFUNC)(const char *&, int *), int border = -1);

inline void read_grid(FILE* fp, std::vector<int>* grid, dimension* dim,
	int border)
{
	read_grid(fp, grid, dim, &read_number, border);
}*/

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

inline void _write_number(FILE* fp, int int_to_write) {
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
	void (*PRINTFUNC)(FILE*, int) = &_write_number, int border = 1);

void write_grid(std::ostream& os, const std::vector<int>& grid, const dimension& dim,
	void (*PRINTFUNC)(char*&, int), int border);

inline void write_grid(FILE* fp, const std::vector<int>* grid, const dimension* dim,
	int border) {
	write_grid(fp, grid, dim, &_write_number, border);
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

/*//! dumps a boost graph into a PDF file, using graphviz
template<class GraphType>
void dump_graph_as_pdf(std::ostream& stream, const GraphType& boost_graph);

//! dumps a boost graph into a PDF file, using graphviz
//! if @a fname is nullptr, the filename is chosen at random
template<class GraphType>
void dump_graph_as_pdf(const GraphType &boost_graph, const char* fname = nullptr);
*/
template<class GraphType>
void dump_graph_as_dot(const GraphType &graph, std::ostream &stream, const boost::dynamic_properties& dp)
{
//	boost::dynamic_properties dp;
//	dp.property("label", boost::get(&Vertex::name, g));
//	dp.property("node_id", get(boost::vertex_index, graph)); //TODO: boost::get?

	// TODO: separate write func?
	write_graphviz_dp(stream, graph, dp);
}

template<class GraphType>
void dump_graph_as_dot(const GraphType &graph, const boost::dynamic_properties& dp, const char* fname = nullptr
	)
{
	const char* filename;
	if(fname)
		filename = fname;
	else
	{
		char temp[] = "graphXXXXXX";
		mkstemp(temp);
		filename = temp;
	}

	std::ofstream stream(filename);
	dump_graph_as_dot(graph, stream, dp);
}

#endif // IO_H

