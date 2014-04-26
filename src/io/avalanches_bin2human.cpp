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
#include "geometry.h" // TODO: only for coord_t -> use types.h?

const unsigned int BUF_SIZE = 1024;

// TODO: specialize for offset = 0 => runtime improvement
template<typename size_each_t>
void parse_avalanches(FILE* in_fp, FILE* out_fp,
	u_coord_t width, std::size_t div_size = 1, std::size_t offset = 0,
	bool ids = false)
{
	size_each_t buffer[BUF_SIZE]; // TODO: was signed... important?
	int last_num_read;
	std::size_t avalanche_number = 1;

	bool do_newline = true;
	bool first_line = true;

	while(!feof(in_fp))
	{
		last_num_read = fread(buffer, sizeof(size_each_t), BUF_SIZE, in_fp);
#ifdef AVALANCHES_DEBUG
		std::cerr << "Reading " << last_num_read << " indexes..." << std::endl;
#endif
		for(int i = 0; i < last_num_read; ++i)
		{
			const size_each_t cur = buffer[i];
			if(cur == -1) {
				do_newline = true;
				++avalanche_number;
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
					 fprintf(out_fp, "%lu",
						avalanche_number);
					do_newline = false;
				}
				// TODO: can we use bitshift for division?
				fprintf(out_fp, " %u", internal2human((cur - offset) / div_size, width));
			}
		}
	}
	if(!first_line)
	 fputs("\n",out_fp);
}

// note: we don't use an out buffer, since stdout will be buffered by \n
// and those \n signs will occur seldom enough...
class MyProgram : public Program
{
	int main()
	{
		bool ids = false;
		u_coord_t width;
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

		struct hdr_info_t
		{
			uint8_t size_each, div_size;
			uint64_t offset;
			void print_info()
			{
#ifdef AVALANCHES_DEBUG
				std::cerr << "Avalanche idx size is " << (int)size_each << std::endl;
				std::cerr << "Size each is " << (int)div_size << std::endl;
				std::cerr << "Index offset is " << offset << std::endl;
#endif
			}
			bool parse(FILE* in_fp)
			{
				char hdr_buf[14];
				fread(hdr_buf, 1, sizeof(hdr_buf), in_fp);
				for(std::size_t i = 0; i < sizeof(hdr_buf); ++i)
				 if(hdr_buf[i] != 0)
				{
					std::cerr << "Byte " << i << " is not a header byte" << std::endl;
					return false;
				}
				fread(&size_each, 1, 1, in_fp);
				fread(&div_size, 1, 1, in_fp);
				fread(&offset, 8, 1, in_fp);
				return true;
			}
		} hdr_info;

		// parse header
		{
			if(!hdr_info.parse(in_fp))
			 exit("Error parsing header");
			hdr_info.print_info();
		}

		// TODO: better use a variadic list to check for 1,2,4,8
		switch(hdr_info.size_each)
		{
			case 1:
				parse_avalanches<int8_t>(in_fp, out_fp, width, hdr_info.div_size, hdr_info.offset, ids);
				break;
			case 2:
				parse_avalanches<int16_t>(in_fp, out_fp, width, hdr_info.div_size, hdr_info.offset, ids);
				break;
			case 4:
				parse_avalanches<int32_t>(in_fp, out_fp, width, hdr_info.div_size, hdr_info.offset, ids);
				break;
			case 8:
				parse_avalanches<int64_t>(in_fp, out_fp, width, hdr_info.div_size, hdr_info.offset, ids);
				break;
			default:
				assert_always(false,
					"The avalanche index size"
					"must be out of {1,2,4,8}.");
		}

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

