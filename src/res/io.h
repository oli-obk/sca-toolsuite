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

#include "general.h"

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


inline int arrow_2_int(char read_char)
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

inline int int_2_arrow(int* int_value)
{
	static const int arrow_palette[4] = { '^', '>', 'v', '<' }; // TODO: ll, tt ?
	//printf("\nint: %d\n", *int_value);
	if(*int_value != *int_value % 4) { // TODO: -3
		std::string error = "Integer could not be converted to arrow: ";
		error += (char)*int_value;
		throw error;
	}
	else return arrow_palette[*int_value];
}

inline bool read_arrow(FILE* fp, int* read_symbol)
{
	char read_char;
	const bool return_value = (fscanf(fp, "%c", &read_char) == 1);
	if(return_value)
	 *read_symbol = arrow_2_int(read_char);
	return return_value;
}

inline bool read_number(FILE* fp, int* read_symbol) {
	return(fscanf(fp, "%d", read_symbol) == 1);
}

/**
	Read a grid from a file pointer (given without border)
	@param fp open file, readable
	@param grid pointer to vector, shall be empty and usually not pre-allocated
	@param dim the real dimension of the grid, i.e. including border
	@param SCANFUNC function which converts chars to numbers for internal handling
	@param border whether the outer cells make a border - internal use only
*/
void read_grid(FILE* fp, std::vector<int>* grid, dimension* dim,
	 bool (*SCANFUNC)(FILE*, int*) = &read_number, bool border = true);

inline void read_array(FILE* fp, std::vector<int>* grid, dimension* dim,
	bool (*SCANFUNC)(FILE*, int*) = &read_number) {
	read_grid(fp, grid, dim, SCANFUNC, false);
}

inline void write_arrow(FILE* fp, int int_to_write) {
	fprintf(fp, "%c", (char)int_2_arrow(&int_to_write));
}

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
	@param border whether the outer cells make a border - internal use only
*/
void write_grid(FILE* fp, const std::vector<int>* grid, const dimension* dim,
	void (*PRINTFUNC)(FILE*, int) = &write_number, bool border = true);

inline void write_array(FILE* fp, const std::vector<int>* grid, const dimension* dim,
	void (*PRINTFUNC)(FILE*, int) = &write_number) {
	write_grid(fp, grid, dim, PRINTFUNC, false);
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

#endif // IO_H

