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

#include <cstdint>
#include <cstdlib>
#include <cstdio>

#include "general.h"
#include "io.h"

const unsigned int BUF_SIZE = 1024;

// note: we don't use an out buffer, since stdout will be buffered by \n
// and those \n signs will occur seldom enough...
class MyProgram : public Program
{
	int main()
	{
		bool ids = false;
		int width;
		switch(argc)
		{
			case 3: assert_usage(!strcmp(argv[2],"ids")); ids = true;
			case 2: width = atoi(argv[1]) + 2;
				break;
			default:
				exit_usage();
				return 1;
		}

		FILE* const in_fp = stdin;
		FILE* const out_fp = stdout;
		char buffer[BUF_SIZE]; // TODO: was signed... important?

		int last_num_read;
		int avalanche_number = 1;
		bool do_newline = true;
		bool first_line = true;

		struct hdr_info_t
		{
			std::size_t size_each;
			uint64_t grid_offset;
		} hdr_info;

		// parse header
		{
			char hdr_buf[7];
			fread(hdr_buf, 1, 7, in_fp);
			for(int i = 0; i < 7; ++i)
			 if(hdr_buf[i] != 0)
				 exit("Input file does not look like an avalanche file.");
			fread(&hdr_info.size_each, 1, 1, in_fp);
			fread(&hdr_info.grid_offset, 8, 1, in_fp);
		}

		while(!feof(in_fp))
		{
			last_num_read = fread(buffer, 4, BUF_SIZE, in_fp);
			for(int i = 0; i < last_num_read; i += hdr_info.size_each)
			{
				const signed int cur = buffer[i];
				if(cur == -1) {
					do_newline = true;
					avalanche_number++;
				}
				else
				{
					if(do_newline)
					{
						if(first_line)
						 first_line = false;
						else
						 fputs("\n", out_fp);
						if(ids)
						 fprintf(out_fp, "%d", avalanche_number);
						do_newline = false;
					}
					fprintf(out_fp, " %d", internal2human(cur, width));
				}
			}
		}
		if(!first_line)
		 fputs("\n",out_fp);
		return (feof(in_fp)!=0)?0:1; // feof==0 <=> stop, but no eof <=> error
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.description = "Converts the binary avalanche output of algorithms in algo into human readable avalanches.\n"
		"Not efficient for large amount of data (code could be improved)";
	help.input = "the binary avalanche data";
	help.output = "the human readable avalanche data (a number sequence)";
	help.syntax = "io/avalanches_bin2human <width> [ids]";
	help.add_param("<width>", "width of grid used to compute the input data");
	help.add_param("ids", "if given, prepends n to the nth avalanche");

	MyProgram program;
	return program.run(argc, argv, &help);
}

