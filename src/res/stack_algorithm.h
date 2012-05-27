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

#include <cassert>
#include <vector>

/**
	@brief Class for stack algorithm using a stack for the avalanches.

	This is the faster way for simulation, but avalanches can not be logged detailled.
	@invariant stack_ptr points to top element
*/
class ArrayStack {
	int* const array; //! first element will never be read: "sentinel"
	int* stack_ptr;
public:
	inline ArrayStack(int human_grid_size) :
		array(new int[human_grid_size+1]), stack_ptr(array) {}
	inline ~ArrayStack() { delete[] array; }
	inline unsigned int pop() { return *(stack_ptr--); }
	inline void push(unsigned int i) { *(++stack_ptr) = i; }
	inline bool empty() const { return stack_ptr == array; }
	inline void flush() const {}
	inline void write_to_file(FILE* fp) const { (void)fp; } // array stack can not do this
	inline void write_separator(FILE* fp) const { (void)fp; }
};

/**
	@brief  Class for stack algorithm using a queue for the avalanches.

	This is the IO efficient way for detailled avalanche logging. If this is not wanted, ArrayStack is faster.
	@invariant write_ptr always points to the element last written
*/
class ArrayQueue {
	int* const array; //! first element will never be read
	int* read_ptr;
	int* write_ptr;
public:
	inline ArrayQueue(int human_grid_size) :
		array(new int[human_grid_size+1]),
		read_ptr(array), write_ptr(array) {}
	inline ~ArrayQueue() { delete[] array; }
	inline unsigned int pop() { return *(++read_ptr); assert(read_ptr <= write_ptr); }
	inline void push(unsigned int i) { *(++write_ptr) = i; }
	inline bool empty() const { return read_ptr == write_ptr; }
	inline void flush() { read_ptr = write_ptr = array; }
	inline void write_to_file(FILE* fp) const { fwrite(array+1, 4, write_ptr-array, fp); }
	inline void write_separator(FILE* fp) const {
		const int minus1 = -1; fwrite(&minus1,4,1, fp);
	}
	inline unsigned int size() { return (unsigned int)(write_ptr-array); }
};

inline void increase_neighbours_without_self(std::vector<int>* grid, const dimension* dim, int center)
{
	(*grid)[center+1]++;
	(*grid)[center-1]++;
	(*grid)[center+dim->width]++;
	(*grid)[center-dim->width]++;
}

/**
	Develops an 1D avalanche. The helping avalanche container is not flushed, so it contains the whole avalanche afterwards.
*/
template<class AvalancheContainer>
inline void avalanche_1d_hint_noflush(std::vector<int>* grid, const dimension* dim, int hint, AvalancheContainer* array, FILE* avalanche_fp)
{
	(*grid)[hint]-=4; // keep up invariant: elements in array are already decreased
	array->push(hint);
	do {
		const int cur_element = array->pop();
		increase_neighbours_without_self(grid, dim, cur_element);

		if((*grid)[cur_element+1] > 3) {
			(*grid)[cur_element+1]&=3;
			array->push(cur_element+1);
		}
		if((*grid)[cur_element-1] > 3) {
			(*grid)[cur_element-1]&=3;
			array->push(cur_element-1);
		}
		if((*grid)[cur_element+dim->width] > 3) {
			(*grid)[cur_element+dim->width]&=3;
			array->push(cur_element+dim->width);
		}
		if((*grid)[cur_element-dim->width] > 3) {
			(*grid)[cur_element-dim->width]&=3;
			array->push(cur_element-dim->width);
		}
	} while( ! array->empty() );
	array->write_to_file(avalanche_fp);
}

/**
	Develops an 1D avalanche. The helping avalanche container is flushed.
*/
template<class AvalancheContainer>
inline void avalanche_1d_hint(std::vector<int>* grid, const dimension* dim, int hint, AvalancheContainer* array, FILE* avalanche_fp)
{
	avalanche_1d_hint_noflush(grid, dim, hint, array, avalanche_fp);
	array->flush(); // note: empty does not always imply being flushed!
}

/**
	Develops an xD avalanche for an x>=0. Writes avalanche seperator afterwards.
	@param times Number of times that the cell at hint may fire. for times=INT_MAX, lx_hint = l_hint
*/
template<class AvalancheContainer>
inline void lx_hint(std::vector<int>* grid, const dimension* dim, int hint, AvalancheContainer* array, FILE* avalanche_fp, int times)
{
	(*grid)[hint]--;
	for(;(*grid)[hint]>2 && times > 0; times--)
	{
		avalanche_1d_hint(grid, dim, hint, array, avalanche_fp);
	}
	array->write_separator(avalanche_fp);
	(*grid)[hint]++;
}

/**
	Develops a full avalanche and writes avalanche seperator afterwards.
	@param array container of type ArrayStack or ArrayQueue.
		ArrayStack is faster (1-2 times), but ArrayQueue can handle IO (instantly!).
*/
template<class AvalancheContainer>
inline void l_hint(std::vector<int>* grid, const dimension* dim, int hint, AvalancheContainer* array, FILE* avalanche_fp)
{
	(*grid)[hint]--;
	while((*grid)[hint]>2)
	{
		avalanche_1d_hint(grid, dim, hint, array, avalanche_fp);
	}
	array->write_separator(avalanche_fp);
	(*grid)[hint]++;
}

template<class AvalancheContainer>
inline void l2_hint(std::vector<int>* grid, const dimension* dim, int hint, AvalancheContainer* array, FILE* avalanche_fp)
{
	(*grid)[hint]--;
	while((*grid)[hint]>2)
	{
		avalanche_1d_hint2(grid, dim, hint, array, avalanche_fp);
	}
	array->write_separator(avalanche_fp);
	(*grid)[hint]++;
}

//! Class for fix algorithm in order to log avalanches binary
class FixLogL {
	FILE* avalanche_fp;
public:
	inline FixLogL(FILE* _avalanche_fp) : avalanche_fp(_avalanche_fp) {}
	inline void write_avalanche_counter() {}
	inline void write_int_to_file(const int* int_ptr, const unsigned int* ntimes) {
		for(unsigned int i = 0; i < *ntimes; i++)
		 fwrite(int_ptr, 4, 1, avalanche_fp);
	}
	inline void write_separator() const {
		const int minus1 = -1; fwrite(&minus1,4,1, avalanche_fp);
	}
};

/*//! Class for fix algorithm in order to log avalanches in human readable format (ASCII text)
class FixLogLHuman {
	FILE* avalanche_fp;
	mutable unsigned int avalanche_counter;
public:
	inline FixLogLHuman(FILE* _avalanche_fp) :
		avalanche_fp(_avalanche_fp),
		avalanche_counter(1) {}
	inline void write_avalanche_counter() {
		fprintf(avalanche_fp, "%u", avalanche_counter);
	}

	inline void write_int_to_file(const int* int_ptr) {
		fprintf(avalanche_fp, " %d", *int_ptr);
	}
	inline void write_separator() const { fputs("\n", avalanche_fp); avalanche_counter++; }
};*/

//! Class for fix algorithm to log nothing. Can be used to output grid afterwards, instead of the avalanche.
class FixLogS {
public:
	inline FixLogS(FILE* fp) { (void) fp; }
	inline void write_avalanche_counter() {}
	inline void write_int_to_file(const int* int_ptr,  const unsigned int* ntimes) {
		(void) int_ptr;
		(void) ntimes;
	}
	inline void write_separator() const {}
};

/**
	This is the alternative algorithm for sandpiles with many more than 3 grains.
	Proposed by Sebastian Frehmel in his Diploma Thesis.
	@param result_logger Class of type FixLogL or FixLogS
*/
template<class AvalancheContainer, class ResultType>
inline void fix(std::vector<int>* grid, const dimension* dim, int hint, AvalancheContainer* array, ResultType* result_logger)
{
	unsigned int fire_times;
	const int INVERT_BIT = (1 << 31);
	const int GRAIN_BITS = (-1) ^ INVERT_BIT;
	if((*grid)[hint]>3)
	{
		array->push(hint);
		result_logger->write_avalanche_counter();

		do {
			const int cur_element = array->pop();
			//result_logger->write_int_to_file(&cur_element);

			(*grid)[cur_element] &= GRAIN_BITS;
			fire_times = (*grid)[cur_element] >>2; // keep up invariant: elements in array are already decreased
			(*grid)[cur_element] -= (fire_times<<2);

			result_logger->write_int_to_file(&cur_element, &fire_times);

			if(((*grid)[cur_element+1]+=fire_times) > 3) {
				(*grid)[cur_element+1] |= INVERT_BIT;
				array->push(cur_element+1);
			}
			if(((*grid)[cur_element-1]+=fire_times) > 3) {
				(*grid)[cur_element-1] |= INVERT_BIT;
				array->push(cur_element-1);
			}
			if(((*grid)[cur_element+dim->width]+=fire_times) > 3) {
				(*grid)[cur_element+dim->width] |= INVERT_BIT;
				array->push(cur_element+dim->width);
			}
			if(((*grid)[cur_element-dim->width]+=fire_times) > 3) {
				(*grid)[cur_element-dim->width] |= INVERT_BIT;
				array->push(cur_element-dim->width);
			}
		} while( ! array->empty() );
	}
	result_logger->write_separator();
	array->flush(); // note: empty does not always imply being flushed!
}


