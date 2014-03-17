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

#include <cstdlib>
#include <cstdio>
#include <vector>

#include "general.h"
#include "io.h"
#include "stack_algorithm.h"

int main(int argc, char** argv)
{
	(void)argv;
	FILE* read_fp = stdin;
	FILE* write_fp = stdout;
	if(argc>1) {
		fputs("There are no arguments for this tool.\n",stderr);
		exit(1);
	}

	unsigned int num_nodes = 0;
	unsigned int width, height;
	fread(&width, 4, 1, read_fp);
	fread(&height, 4, 1, read_fp);
	num_nodes = (width-2) * (height-2);
	for(unsigned int i=1; i<=num_nodes;i++)
	 fprintf(write_fp, "%d v_%d\n",i,i);
/*	do {
		if( fread(&current_int, 4, 1, read_fp) > 0 && current_int == -1 ) {
			num_nodes++;
			fprintf(write_fp, "%d v_%d\n",num_nodes,num_nodes);
		}
	} while(!feof(read_fp));*/

	fprintf(write_fp, "#\n");

	rewind(read_fp);

	unsigned int current_node = 1;
	int current_int;
	while(current_node <= num_nodes) {
//printf("ftell: %d\n", ftell(read_fp));
		assert( fread(&current_int, 4, 1, read_fp) > 0 );
		//printf("current_int: %d\n", current_int);
//printf("ftell: %d\n", ftell(read_fp));
		if(current_int == -1)
		{
			current_node++;
		}
		else
		{
			const unsigned int to_node = internal2human(current_int,width)+1;
			if(to_node != current_node) // avoid loops
			 fprintf(write_fp, "%d %d\n", current_node, to_node);
		}
	}

	return 0;
}

