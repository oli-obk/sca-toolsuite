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

#ifndef IMAGE_H
#define IMAGE_H

#include <cstdio>
#include <cstring>

#include "general.h"

class rgb
{
	inline static int int_from_2_hex(const char* str)
	{
		int value; sscanf(str, "%2x", &value);
		return value;
	}

	short r, g, b; // use short to handle range [-255,255]
public:
	inline void to_24bit(char* result) const {
		result[0]=b; result[1]=g; result[2]=r;
	}
	inline rgb operator-(const rgb& other) const {
		return rgb(r-other.r, g-other.g, b-other.b);
	}
	inline rgb operator/(const char& divisor) const {
		return rgb(r/divisor, g/divisor, b/divisor);
	}
	inline rgb& operator+=(const rgb& other) {
		r+=other.r; g+=other.g; b+=other.b;
		return *this;
	}
	void from_str(const char* color_str);
	rgb() {}
	inline rgb(const char* color_str) { from_str(color_str); }
	rgb(int _r, int _g, int _b) : r(_r), g(_g), b(_b) {}
	void print(FILE* fp) const {
		fprintf(fp, "red: %d, green: %d, blue: %d\n",r,g,b);
	}
};

class ColorTable
{
	rgb min_color, max_color, step_size;
	int min_val, max_val;
public:
	class const_iterator
	{
		const ColorTable& ct;
		rgb cur_color;
		int cur_val;
	public:
		const_iterator(const ColorTable& _ct)
			: ct(_ct), cur_color(ct.min_color), cur_val(ct.min_val)
		{}
		const const_iterator& operator++();
		inline bool valid() { return cur_val <= ct.max_val; }
		const rgb* operator->() const { return &cur_color; }
	};

	inline int num_colors() const { return max_val - min_val + 1; }
	inline int index_2_ct_index(int index) const {
		if(index < min_val || index > max_val)
		 throw("Error: Found pixel out of range [min_val, max_val]");
		return index - min_val;
	}
	ColorTable(rgb _min_color, rgb _max_color, int _min_val, int _max_val);
};

namespace tga
{
	void print_header(FILE* fp, const dimension& dim, const ColorTable& ct);
	void print_color_map(FILE* fp, const ColorTable& ct);
	void print_image_map(FILE* fp, const std::vector<int>& grid,
		const ColorTable& ct);
	void print_footer(FILE* fp);
}

void print_to_tga(FILE* fp, const ColorTable& ct,
	const std::vector<int>& grid, const dimension& dim);

#endif // IMAGE_H
