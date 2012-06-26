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

#include "image.h"

void rgb::from_str(const char* color_str)
{
	if(strlen(color_str)!=6)
	 throw("Error: Color strings shall be rrggbb");
	r = int_from_2_hex(color_str);
	g = int_from_2_hex(color_str+2);
	b = int_from_2_hex(color_str+4);
}

const ColorTable::const_iterator& ColorTable::const_iterator::operator++()
{
	cur_color += ct.step_size;
	cur_val++;
	return *this;
}

ColorTable::ColorTable(rgb _min_color, rgb _max_color, int _min_val, int _max_val)
	: min_color(_min_color), max_color(_max_color),
	min_val(_min_val), max_val(_max_val)
{
	if(min_val != max_val)
	 step_size = (max_color - min_color)/(max_val-min_val);
}

void tga::print_header(FILE* fp, const dimension& dim, const ColorTable& ct)
{
	const char BITS_PER_COLOR = 24;
	const char BIT_PER_PIXEL = 8; // GIMP does not allow 16

	// 0-0: no image id field
	// 1-1: color map? yes
	// 2-2: image type: color mapped
	fwrite("\x00\x01\x01", 1, 3, fp);

	// color map:
	// 0-1: index of first entry (=0)
	// 2-3: number of entries
	// 4-4: bpp to describe each color
	fwrite("\x00\x00", 1, 2, fp);
	const short number_of_colors = ct.num_colors();
	fwrite(&number_of_colors, 1, 2, fp);
	fwrite(&BITS_PER_COLOR, 1, 1, fp);

	// 0-1: x-origin (=0)
	// 2-3: y-origin (=0)
	fwrite("\x00\x00\x00\x00", 1, 4, fp);

	const short width = dim.width - 2;
	fwrite(&width, 2, 1,  fp);
	const short height = dim.height - 2;
	fwrite(&height, 2, 1, fp);

	// 0: pixel depth (=BPP)
	// 1: bits 3-0: attributes per pixel (=alpha?) =0
	// 1: bits 4-5: left-right => set bit 4
	// 1: bits 6-7: reserved to be 0
	fwrite(&BIT_PER_PIXEL, 1, 1, fp);
	fwrite("\x20", 1, 1, fp);

}

void tga::print_color_map(FILE* fp, const ColorTable& ct)
{
	// (image id: empty)
	for(ColorTable::const_iterator itr(ct); itr.valid(); ++itr)
	{
		char bitmask[3];
		itr->to_24bit(bitmask);
		fwrite(bitmask, 1, 3, fp);
	}
}

void tga::print_image_map(FILE* fp, const std::vector<int>& grid,
	const ColorTable& ct)
{
	for(std::vector<int>::const_iterator itr = grid.begin();
		itr != grid.end(); ++itr)
	{
		if(*itr != INT_MIN)
		{
			const char index = ct.index_2_ct_index(*itr);
			fwrite(&index, 1, 1, fp);
		}
	}
}

void tga::print_footer(FILE* fp)
{
	fwrite("\x00\x00\x00\x00",1,4,fp); // no offset
	fwrite("\x00\x00\x00\x00",1,4,fp); // no offset
	fwrite("TRUEVISION-XFILE.\x00",1,18,fp);
}

void print_to_tga(FILE* fp, const ColorTable& ct,
	const std::vector<int>& grid, const dimension& dim)
{
	tga::print_header(fp, dim, ct);
	tga::print_color_map(fp, ct);
	tga::print_image_map(fp, grid, ct);
	tga::print_footer(fp);
}

