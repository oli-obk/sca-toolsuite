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

#ifndef ROTOR_ALGORITHM_H
#define ROTOR_ALGORITHM_H

#include <stack_algorithm.h>

const int INVERT_BIT = (1 << 31);

template<class AvalancheContainer>
inline void inc_push_neighbour(std::vector<int>* chips, int position, AvalancheContainer* array)
{
	++(*chips)[position];
	//if((*chips)[position]>=0)
	{
		(*chips)[position] |= INVERT_BIT;
		array->push(position);
	}
}

template<class AvalancheContainer>
inline void inc_push_neighbour(std::vector<int>* chips, int position, AvalancheContainer* array, int n)
{
	(*chips)[position] += n;
	//if((*chips)[position]>=0)
	{
		(*chips)[position] |= INVERT_BIT;
		array->push(position);
	}
}

template<class AvalancheContainer, class ResultType>
inline void rotor_fix(std::vector<int>* grid, std::vector<int>* chips,
	const dimension* dim, int hint, AvalancheContainer* array, ResultType* result_logger)
{
	const int INVERT_BIT = (1 << 31);
	const int GRAIN_BITS = (-1) ^ INVERT_BIT;

	const int PALETTE[7] = { -dim->width, 1, dim->width, -1, -dim->width, 1, dim->width };

	if((*chips)[hint]>0) // otherwise, we would need an additional "case 0" label
	{
		array->push(hint);
		result_logger->write_avalanche_counter();

		do {
			const int cur_element = array->pop();
			//result_logger->write_int_to_file(&cur_element); // TODO

			(*chips)[cur_element] &= GRAIN_BITS;
			int cur_rotor = (*grid)[cur_element];


			int chips_to_dec = (*chips)[cur_element];
			int chips_to_dec_4 = chips_to_dec >> 2;

			if(chips_to_dec_4)
			{
				inc_push_neighbour(chips, cur_element - dim->width, array, chips_to_dec_4);
				inc_push_neighbour(chips, cur_element - 1, array, chips_to_dec_4);
				inc_push_neighbour(chips, cur_element + 1, array, chips_to_dec_4);
				inc_push_neighbour(chips, cur_element + dim->width, array, chips_to_dec_4);
			}
			//int chips_to_dec = std::min(3,(*chips)[cur_element]);

			chips_to_dec &= 3;

			//printf("%d chips, rotor: %d\n", (*chips)[cur_element], (*grid)[cur_element]);
			switch(chips_to_dec) // assumption: more will no hapen
			{
				// unrolled for loop
				case 3: ++cur_rotor; inc_push_neighbour(chips, cur_element + PALETTE[cur_rotor], array);
				case 2: ++cur_rotor; inc_push_neighbour(chips, cur_element + PALETTE[cur_rotor], array);
				case 1: ++cur_rotor; inc_push_neighbour(chips, cur_element + PALETTE[cur_rotor], array); break;
				default: ;
				/*	default: {
					std::string err_str = "Invalid number of chips detected."; // TODO: number
					throw(err_str);
				}*/
			}
			(*grid)[cur_element] = ((*grid)[cur_element] + chips_to_dec)%4;
			if( ((*chips)[cur_element] -= chips_to_dec) )
			 array->push(cur_element);
		} while( ! array->empty() );
	}
	result_logger->write_separator();
	array->flush(); // note: empty does not always imply being flushed!
}

template<class AvalancheContainer, class ResultType>
inline void rotor_fix_naive(std::vector<int>* grid, std::vector<int>* chips,
	const dimension* dim, AvalancheContainer* array, ResultType* result_logger)
{
	const int PALETTE[4] = { -dim->width, 1, dim->width, -1 };
	const unsigned int ONCE = 1;
	//unsigned int area = dim->area();
	int cur_field;
	for(unsigned int y = 1; y<(dim->height-1); y++)
	for(unsigned int x = 1; x<(dim->width-1); x++)
	{
		unsigned int coord = (y*dim->width)+x;
		//if(is_border(dim, (y*dim->width)+x))
		//	exit(99);
		for(int nchips = (*chips)[coord]; nchips; --nchips)
		{
			cur_field = coord;
			do
			{
			//	printf("cur field: %d\n",cur_field);
				result_logger->write_int_to_file(&cur_field, &ONCE);
				cur_field += PALETTE[(++(*grid)[cur_field])%=4];
			}
			while((*grid)[cur_field]>=0); // = not border
		}
	}
	result_logger->write_separator();
}

#endif // ROTOR_ALGORITHM_H
