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

#include <vector>
#include <cctype> // isdigit()
#include <iostream>
#include <cstring>

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

class base_grid
{
public:
	virtual void write(char*& ptr, int int_to_write) const = 0;
	virtual void read(const char*& ptr, int* read_symbol) const = 0;
	virtual const char* name() const = 0;
};

class number_grid : public base_grid
{
	inline static bool is_number_parsable(char sgn)
	{
		return isdigit(sgn) || (sgn == '-');
	}
public:
	inline void write(char*& ptr, int int_to_write) const {
		ptr += sprintf(ptr, "%d", int_to_write);
	}

	inline void read(const char*& ptr, int* read_symbol) const {
		*read_symbol = atoi(ptr);
		while(is_number_parsable(*++ptr));
	}

	const char* name() const { return "numbers"; }
};

class arrow_grid : public base_grid
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
				char error[] = "Invalid arrow sign read:  ";
				error[strlen(error-3)] = read_char;
				throw error;
			}
		}
	}

	inline static char int_2_arrow(const int* int_value)
	{
		static const int arrow_palette[4] = { '^', '>', 'v', '<' }; // TODO: ll, tt ?
		if(*int_value != *int_value % 4) { // TODO: -3
			const char* error = "Integer could not be converted to arrow.";
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
public:
	inline void write(char*& ptr, int int_to_write) const {
//		ptr += sprintf(ptr, "%d", int_to_write);
		*(ptr++) = int_2_arrow(&int_to_write);
	}

	inline void read(const char*& ptr, int* read_symbol) const {
		//*read_symbol = atoi(ptr);
		//while(is_number_parsable(*++ptr));
		*read_symbol = arrow_2_int(*(ptr++));
	}


	const char* name() { return "rotors"; }
};

/*
template<class ...TypeList>
void read_file(const char* type_name)
{

}*/

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

//void read_grid(std::istream& is, std::vector<int>& grid, dimension& dim,
//	void (*SCANFUNC)(const char *&, int *), int border = -1);

inline void read_grid(FILE* fp, std::vector<int>* grid, dimension* dim,
	int border)
{
	read_grid(fp, grid, dim, &read_number, border);
}

void read_grid(const base_grid* grid_class, std::istream& is, std::vector<int>& grid, dimension& dim,
	int border = -1);

template<class GridType = number_grid>
void read_grid(std::istream& is, std::vector<int>& grid, dimension& dim, int border = -1) {
	GridType grid_class;
	read_grid(&grid_class, is, grid, dim, border);
}



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

//void write_grid(std::ostream& os, const std::vector<int>& grid, const dimension& dim,
//	void (*PRINTFUNC)(char*&, int), int border);

void write_grid(const base_grid* grid_class, std::ostream& os, const std::vector<int>& grid, const dimension& dim,
	int border);

inline void write_grid(FILE* fp, const std::vector<int>* grid, const dimension* dim,
	int border) {
	write_grid(fp, grid, dim, &_write_number, border);
}

template<class GridType = number_grid>
void write_grid(std::ostream& os, const std::vector<int>& grid, const dimension& dim,
	int border)
{
	GridType grid_class;
	write_grid(&grid_class, os, grid, dim, border);
}


/**
	Executes shell command and pipes output to stdout.
	Useful to get more than one input into your program.
	@param shell_command Command to be executed with sh -c. Can contain any shell syntax.
	@return true iff new process could be started
*/
bool get_input(const char* shell_command);

#endif // IO_H

