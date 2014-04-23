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

#ifndef STACK_ALGORITHM_H
#define STACK_ALGORITHM_H

#include <cassert>
#include <cstdint>
#include <vector>
#include "geometry.h"

namespace sandpile
{

/**
	@brief Class for stack algorithm using a stack for the avalanches.

	This is the faster way for simulation, but avalanches can not be logged detailled.
	@invariant stack_ptr points to top element
*/
template<class T = int*>
class _array_stack
{
	T* const array; //! first element will never be read: "sentinel"
	T* stack_ptr;
public:
	using value_type = T;
	inline _array_stack(unsigned human_grid_size) :
		array(new T[human_grid_size+1]), stack_ptr(array) {}
	inline ~_array_stack() { delete[] array; }
	inline T pop() { return *(stack_ptr--); }
	inline void push(const T& i) { *(++stack_ptr) = i; }
	inline bool empty() const { return stack_ptr == array; }
	inline void flush() const {}
	inline void write_to_file(FILE* ) const {} // array stack can not do this
	inline void write_separator(FILE* ) const {}
	inline void write_header(FILE* , uint64_t ) const {}
};

typedef _array_stack<int*> array_stack;

/**
	@brief  Class for stack algorithm using a queue for the avalanches

	This is the way you should go if you are interested in the detailled avalanches.
	They are stored in the array right after the algorithm. If you need to store the
	avalanches in a file, look for array_queue instead.
	@invariant write_ptr always points to the element last written
*/
template<class T = int*>
class _array_queue_no_file
{
protected:
	T* const array; //! first element will never be read
	T* read_ptr;
	T* write_ptr;
public:
	using value_type = T;
	inline _array_queue_no_file(unsigned human_grid_size) :
		array(new T[human_grid_size+1]),
		read_ptr(array), write_ptr(array) {}
	inline ~_array_queue_no_file() { delete[] array; }
	inline T pop() { return *(++read_ptr); assert(read_ptr <= write_ptr); }
	inline void push(const T& i) { *(++write_ptr) = i; }
	inline bool empty() const { return read_ptr == write_ptr; }
	inline void flush() { read_ptr = write_ptr = array; }
	inline void write_to_file(FILE* ) const {}
	inline void write_separator(FILE* ) const {}
	inline void write_header(FILE* , uint64_t ) const {}
	inline unsigned int size() { return (unsigned int)(write_ptr-array); }
	inline const T* data() const { return array+1; }
};

typedef _array_queue_no_file<int*> array_queue_no_file;

/**
	@brief Class for stack algorithm (and IO) using a queue for the avalanches.

	This is the IO efficient way for detailled avalanche logging in files.
	If this is not wanted, array_stack is faster.
	@invariant write_ptr always points to the element last written
*/
template<class T>
class _array_queue : public _array_queue_no_file<T>
{
	// TODO: this class works incorrect with pointers!
	typedef _array_queue_no_file<T> base;
public:
	inline _array_queue(unsigned human_grid_size) : base(human_grid_size) {}
	inline void write_to_file(FILE* fp) const {
		fwrite(base::array+1, sizeof(T), base::write_ptr - base::array, fp);
	}
	inline void write_separator(FILE* fp) const {
		const int minus1 = -1; fwrite(&minus1, sizeof(T), 1, fp);
	}
	inline void write_header(FILE* fp, uint64_t grid_offset) const
	{
		constexpr static const char sizeof_t = sizeof(T);
		constexpr static const char hdr[7] = { 0, 0, 0, 0, 0, 0, 0 };
		fwrite(&hdr, 7, 1, fp);
		fwrite(&sizeof_t, 1, 1, fp);
		fwrite(&grid_offset, sizeof(uint64_t), 1, fp);
	}
};

typedef _array_queue<int*> array_queue;

/*inline void increase_neighbours_without_self(std::vector<int>& grid, const unsigned grid_width, const int center)
{
	grid[center+1]++;
	grid[center-1]++;
	grid[center+grid_width]++;
	grid[center-grid_width]++;
}*/

/**
	Develops an 1D avalanche. The helping avalanche container is not flushed, so it contains the whole avalanche afterwards.
	Important: The cell at hint must be decreased by 1.
*/
template<class T, class AvalancheContainer>
inline void avalanche_1d_hint_noflush(std::vector<T>& grid, const signed grid_width, const std::size_t hint, AvalancheContainer& array, FILE* const avalanche_fp)
{
	grid[hint]-=4; // keep up invariant: elements in array are already decreased
	array.push(&grid[hint]);
	do
	{
		using vt = typename AvalancheContainer::value_type;
		vt const ptr = array.pop();

		vt const ptr1 = ptr + 1;
		if(++*ptr1 > 3) {
			*ptr1&=3;
			array.push(ptr1);
		}
		vt const ptr_1 = ptr - 1;
		if(++*ptr_1 > 3) {
			*ptr_1&=3;
			array.push(ptr_1);
		}
		vt const ptr_2 = ptr + grid_width;
		if(++*ptr_2 > 3) {
			*ptr_2&=3; // TODO: template variant with ==4 => = 0 ?
			array.push(ptr_2);
		}

		vt const ptr_3 = ptr - grid_width;
		if(++*ptr_3 > 3) {
			*ptr_3&=3;
			array.push(ptr_3);
		}

	} while( ! array.empty() );
	array.write_to_file(avalanche_fp);
}

template<class T, class AvalancheContainer>
inline void avalanche_1d_hint_noflush(std::vector<T>* grid, const dimension* dim, const std::size_t hint, AvalancheContainer* array, FILE* avalanche_fp)
{
	avalanche_1d_hint_noflush(*grid, dim->width(), hint, *array, avalanche_fp);
}

/**
	Develops an 1D avalanche. The helping avalanche container is flushed.
	Important: The cell at hint must be decreased by 1.
*/
template<class T, class AvalancheContainer>
inline void avalanche_1d_hint(std::vector<T>* grid, const dimension* dim, const std::size_t hint, AvalancheContainer* array, FILE* avalanche_fp)
{
	avalanche_1d_hint_noflush(grid, dim, hint, array, avalanche_fp);
	array->flush(); // note: empty does not always imply being flushed!
}

/**
	Develops an xD avalanche for an x>=0. Writes avalanche seperator afterwards.
	@param times Number of times that the cell at hint may fire. for times=INT_MAX, lx_hint = l_hint
*/
template<class T, class AvalancheContainer>
inline void lx_hint(std::vector<T>* grid, const dimension* dim, const std::size_t hint, AvalancheContainer* array, FILE* avalanche_fp, int times)
{
	array->write_header(avalanche_fp, (uint64_t)grid->data());
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
	@param array container of type array_stack or array_queue.
		array_stack is faster (1-2 times), but array_queue can handle IO (instantly!).
*/
template<class T, class AvalancheContainer>
inline void l_hint(std::vector<T>* grid, const dimension* dim, const std::size_t hint, AvalancheContainer* array, FILE* avalanche_fp)
{
	array->write_header(avalanche_fp, (uint64_t)grid->data());
	(*grid)[hint]--;
	while((*grid)[hint]>2)
	{ // TODO: fit this for char types: all 128 rounds (lx_hint?)
		avalanche_1d_hint(grid, dim, hint, array, avalanche_fp);
	//	avalanche_1d_hint_noflush_2<int>(grid, dim, hint/*, array, avalanche_fp*/);
	}
	array->write_separator(avalanche_fp);
	(*grid)[hint]++;
}

#if 0
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
#endif

//! Class for fix algorithm in order to log avalanches binary
class fix_log_l
{
	using T = int*;
	FILE* avalanche_fp;
public:
	inline fix_log_l(FILE* _avalanche_fp) : avalanche_fp(_avalanche_fp) {}
//	inline void write_avalanche_counter() {}
	inline void write_int_to_file(const int* int_ptr, const unsigned int* ntimes) {
		for(unsigned int i = 0; i < *ntimes; i++)
		 fwrite(&int_ptr, 4, 1, avalanche_fp);
	}
	inline void write_separator() const {
		const int minus1 = -1; fwrite(&minus1,4,1, avalanche_fp);
	}

	inline void write_header(uint64_t grid_offset) const
	{
		constexpr static const char sizeof_t = sizeof(T);
		constexpr static const char hdr[7] = { 0, 0, 0, 0, 0, 0, 0 };
		fwrite(&hdr, 7, 1, avalanche_fp);
		fwrite(&sizeof_t, 1, 1, avalanche_fp);
		fwrite(&grid_offset, sizeof(uint64_t), 1, avalanche_fp);
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
class fix_log_s
{
public:
	inline fix_log_s(FILE* fp) { (void) fp; }
//	inline void write_avalanche_counter() {}
	inline void write_int_to_file(const int* int_ptr,  const unsigned int* ntimes) {
		(void) int_ptr;
		(void) ntimes;
	}
	inline void write_separator() const {}
	inline void write_header(uint64_t ) const {}
};

template<class AvalancheContainer, class ResultType>
inline void do_fix(/*std::vector<int>* grid,*/ const dimension* dim, AvalancheContainer* array, ResultType* result_logger)
{
	unsigned int fire_times;
	const int INVERT_BIT = (1 << 31);
	const int GRAIN_BITS = (-1) ^ INVERT_BIT;

//	result_logger->write_avalanche_counter();

	do
	{
		using vt = typename AvalancheContainer::value_type;
		vt const cur_element = array->pop(); // TODO!!
		//printf("cur: %d\n",cur_element);

		*cur_element &= GRAIN_BITS;
		fire_times = *cur_element >>2; // keep up invariant: elements in array are already decreased
		*cur_element -= (fire_times<<2);

		result_logger->write_int_to_file(cur_element, &fire_times);

		vt const e = cur_element + 1;
		if((*e+=fire_times) > 3) {
			*e |= INVERT_BIT;
			array->push(e);
		}
		vt const w = cur_element-1;
		if((*w+=fire_times) > 3) {
			*w |= INVERT_BIT;
			array->push(w);
		}
		vt const s = cur_element + dim->width();
		if((*s+=fire_times) > 3) {
			*s |= INVERT_BIT;
			array->push(s);
		}
		vt const n = cur_element - dim->width();
		if((*n+=fire_times) > 3) {
			*n |= INVERT_BIT;
			array->push(n);
		}

	} while( ! array->empty() );
}

/**
	This is the alternative algorithm for sandpiles with many more than 3 grains.
	Proposed by Sebastian Frehmel in his Diploma Thesis.
	@param result_logger Class of type FixLogL or FixLogS
*/
template<class AvalancheContainer, class ResultType>
inline void fix(std::vector<int>* grid, const dimension* dim, int hint, AvalancheContainer* array, ResultType* result_logger)
{
	//printf("hint: %d\n",hint);
	result_logger->write_header((uint64_t)grid->data());
	if((*grid)[hint]>3)
	{
		array->push(&((*grid)[hint]));
		do_fix(dim, array, result_logger);
	}
	result_logger->write_separator();
	array->flush(); // note: empty does not always imply being flushed!
}

//! version without a hint
template<class AvalancheContainer, class ResultType>
inline void fix(std::vector<int>* grid, const dimension* dim, AvalancheContainer* array, ResultType* result_logger)
{
	const int INVERT_BIT = (1 << 31);

	result_logger->write_header((uint64_t)grid->data());
	for(unsigned int count = 0; count < dim->area(); ++count)
	{
		if(! is_border(dim, count)) {
			// TODO: improve this!
			array->push(&((*grid)[count])); // panic: every cell is assumed to be higher than 3
			(*grid)[count] |= INVERT_BIT; // don't push this one twice
		}
	}

	// note: hint does not matter for correctness
	do_fix(dim, array, result_logger);
}

}

#endif // STACK_ALGORITHM_H
