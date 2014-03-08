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

#include <climits>
#include <cstdio>

// for get_input:
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>

#include "io.h"

inline void insert_horizontal_border(std::vector<int>* grid,
	std::vector<int>::iterator itr,
	int human_linewidth,
	int border_width)
{
	grid->insert(itr,
		(human_linewidth + (border_width<<1))*border_width,
		INT_MIN);
}

inline void insert_vertical_border(std::vector<int>* grid,
	std::vector<int>::iterator itr,
	int border_width)
{
	grid->insert(itr, border_width, INT_MIN);
}

/*
 * Shall we use width for a line including or excluding borders?
 * convention: The user (and even an AI random inputter)
 * 	can not type -infinity = INT_MIN, so keep width/height for them
 * 	as excluding border.
 * The real model, however, requires dead elements on the border, they
 * *do* belong to the model. Thus, internally, we always include the border
 */

constexpr size_t buffer_size = 4096; // TODO: cmake
char read_buffer[buffer_size];

// reads grid from open file
// TODO: we should reserve the array linewise...
void read_grid(FILE* fp, std::vector<int>* grid, dimension* dim,
	void (*SCANFUNC)(const char *&, int *), int border)
{
	assert(SCANFUNC);

	int read_symbol;
	int line_width = -1, col_count = 0, line_count = 0; // all excl. border

	while(true)
	{
		const char* ptr = fgets(read_buffer, buffer_size, fp);
		if(ptr == nullptr // eof
			|| *ptr == '\n') // empty line = abort
			break;

		do
		{
			// scan symbol
			SCANFUNC(ptr, &read_symbol);
			if(!col_count) // (TODO: move this somewhere else?)
			 insert_vertical_border(grid, grid->end(), border);
			grid->push_back(read_symbol);
			col_count++;

			// read separating whitespace
			read_symbol=*ptr++;
			if(read_symbol == '\n')
			{
				// first newline => determine line length
				if(! line_count) {
					line_width = col_count;
					insert_horizontal_border(grid, grid->begin(), line_width, border);
				}
				else
				 assert(line_width == col_count);

				line_count++;
				col_count = 0;

				insert_vertical_border(grid, grid->end(), border);
			}
			else
			 assert(read_symbol == ' ');

//			printf("ptr now: '%s'\n", ptr);
//			printf("ptr is EOF? %d\n",read_symbol==EOF);

		} while(read_symbol != '\n' && read_symbol != EOF);


#if 0
		// read first numeric
		if(! SCANFUNC(fp, &read_symbol) )
		 break; // eof
		if(!col_count)
		 insert_vertical_border(grid, grid->end(), border);
		grid->push_back(read_symbol);
		col_count++;

		// read separating whitespace
		read_symbol=fgetc(fp);
		if(read_symbol == '\n')
		{
			// first newline => determine line length
			if(! line_count) {
				line_width = col_count;
				insert_horizontal_border(grid, grid->begin(), line_width, border);
			}
			else
			 assert(line_width == col_count);

			line_count++;
			col_count = 0;

			insert_vertical_border(grid, grid->end(), border);
		}
		else
		 assert(read_symbol == ' ');
#endif
	}

	insert_horizontal_border(grid, grid->end(), line_width, border);

	dim->height = line_count + (((int)(border))<<1);
	dim->width = line_width + (((int)(border))<<1);

	assert(dim->area() == grid->size());
}

void write_grid(FILE* fp, const std::vector<int>* grid, const dimension* dim,
	void (*PRINTFUNC)(FILE*, int), int border)
{
	assert(PRINTFUNC);
	unsigned int last_symbol = dim->width - 1 - border;

	for(unsigned int y = border; y < dim->height - border; y++)
	{
		for(unsigned int x = border; x < dim->width - border; x++) {
			PRINTFUNC(fp, (*grid)[x + (dim->width)*y]); // TODO: two [] operators
			fputc((x == last_symbol) ? '\n':' ', fp);
		}
	}
}

void create_boost_graph(FILE* read_fp, graph_t* boost_graph)
{

	unsigned int num_nodes = 0;
	unsigned int width, height;
	fread(&width, 4, 1, read_fp);
	fread(&height, 4, 1, read_fp);
	num_nodes = (width-2) * (height-2);
	for(unsigned int i=0; i<num_nodes;i++)
	 boost::add_vertex(*boost_graph);
//	 fprintf(write_fp, "%d v_%d\n",i,i);


//	fprintf(write_fp, "#\n");

	unsigned int current_node = 0;
	int current_int;
	while(current_node < num_nodes)
	{
		{
			const int num_read = fread(&current_int, 4, 1, read_fp);
			assert(num_read > 0);
		}
		if(current_int == -1)
		{
			current_node++;
		}
		else
		{
			const unsigned int to_node = internal2human(current_int,width);
			if(to_node != current_node) // avoid loops
			 boost::add_edge(current_node, to_node, *boost_graph);
			//printf("read edge %d %d\n",current_node, to_node);
			// fprintf(write_fp, "%d %d\n", current_node, to_node);
		}
	}
}

void dump_graph_as_tgf(FILE* write_fp, const graph_t* graph)
{
	// dump nodes
	for(unsigned int i=1; i<=num_vertices(*graph);i++)
	 fprintf(write_fp, "%d v_%d\n",i,i);

	// get the property map for vertex indices
	typedef property_map<graph_t, vertex_index_t>::type IndexMap;
	IndexMap index = get(vertex_index, *graph);

	// dump seperator
	fprintf(write_fp, "#\n");

	// dump edges
	graph_traits<graph_t>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(*graph); ei != ei_end; ++ei)
	{
		const int cur_source = index[source(*ei, *graph)];
		const int cur_target = index[target(*ei, *graph)];
		fprintf(write_fp, "%d %d\n", cur_source+1, cur_target+1);
	}
}

bool get_input(const char* shell_command)
{
	int pipefd[2];
	pid_t childs_pid;

	if (pipe(pipefd) == -1) {
		puts("... pipe() did not work, no sh!");
		return false;
	}

//	fcntl(pipefd[0], F_SETFL, O_NONBLOCK); // ?????

	// fork sh
	childs_pid=fork();
	if(childs_pid < 0) {
		puts("... fork() did not work, no sh!");
		return false;
	}
	else if(childs_pid == 0) {

		close(pipefd[0]); /* Close unused read end */

		dup2(pipefd[1], STDOUT_FILENO);

		execlp("/bin/sh", "sh"	, "-c", shell_command, NULL);

		close(pipefd[1]); /* Reader will see EOF */
		exit(0);
	}

	close(pipefd[1]); /* Close unused write end */
	dup2(pipefd[0], STDIN_FILENO);
	return true;
}

