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
#include <type_traits>
#include "geometry.h"

namespace sandpile
{

/*
 * io logging classes
 */
template<class T>
class log_base
{
	//! difference of two grid indices for non pointers
	template<class >
	class div_size_t
	{
		const uint8_t _size = 1;
	public:
		const uint8_t& size() const { return _size; }
	};

	//! difference of two grid indices for pointers
	template<class T2>
	class div_size_t<T2*>
	{
		using rpt = typename std::remove_pointer<T2>::type;
		const uint8_t _size = sizeof(rpt);
	public:
		const uint8_t& size() const { return _size; }
	};

	const div_size_t<T> div_size;
	FILE* fp;
public:
	log_base(FILE* fp) : div_size(), fp(fp) {}
	inline void write_separator() const {
		const uint64_t minus1 = -1; fwrite(&minus1, sizeof(T), 1, fp);
	}
	inline void write_header(uint64_t grid_offset) const
	{
		constexpr static const char sizeof_t = sizeof(T);
		constexpr static const char hdr[14] = {}; // initialized to 0
		fwrite(&hdr, sizeof(hdr), 1, fp);
		fwrite(&sizeof_t, 1, 1, fp); // size of each index
		fwrite(&div_size.size(), 1, 1, fp); // diff between two index numbers
		fwrite(&grid_offset, sizeof(uint64_t), 1, fp);
	}
	inline void write_elem_to_file(const T elem, const uint32_t* const ntimes) {
		for(uint32_t i = 0; i < *ntimes; i++)
		 fwrite(&elem, sizeof(T), 1, fp);
	}
	inline void write_array_to_file(T* const ptr, const int num) const {
		fwrite(ptr, sizeof(T), num, fp);
	}
};

template<class T>
class log_nothing_base
{
//	inline void write_to_file(FILE* ) const {} // array stack can not do this
public:
	log_nothing_base() {}
	log_nothing_base(FILE* ) {}
	inline void write_separator() const {}
	inline void write_header(uint64_t ) const {}
	inline void write_elem_to_file(const T, const uint32_t*) const {}
};

/*
 * array storage classes
 */
/**
	@brief Class for stack algorithm using a stack for the avalanches.

	This is the faster way for simulation, but avalanches can not be logged detailled.
	@invariant stack_ptr points to top element
*/
template<class T = int*>
class _array_stack : public log_nothing_base<T>
{
	T* const array; //! first element will never be read: "sentinel"
	T* stack_ptr;
public:
	using value_type = T;
	inline _array_stack(unsigned human_grid_size, FILE* = nullptr) :
		array(new T[human_grid_size+1]), stack_ptr(array) {}
	inline ~_array_stack() { delete[] array; }
	inline T pop() { return *(stack_ptr--); }
	inline void push(const T& i) { *(++stack_ptr) = i; }
	inline bool empty() const { return stack_ptr == array; }
	inline void flush() const {}

	// logging:
	inline void write_to_file() const {}
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
class _array_queue_base
{
protected:
	T* const array; //! first element will never be read
	T* read_ptr;
	T* write_ptr;
public:
	using value_type = T;
	inline _array_queue_base(unsigned human_grid_size) :
		array(new T[human_grid_size+1]),
		read_ptr(array), write_ptr(array) {}
	inline ~_array_queue_base() { delete[] array; }
	inline T pop() { return *(++read_ptr); assert(read_ptr <= write_ptr); }
	inline void push(const T& i) { *(++write_ptr) = i; }
	inline bool empty() const { return read_ptr == write_ptr; }
	inline void flush() { read_ptr = write_ptr = array; }
	inline unsigned int size() { return (unsigned int)(write_ptr-array); }
	inline const T* data() const { return array+1; }
};

/**
	@brief Class for stack algorithm (and IO) using a queue for the avalanches.

	This is the IO efficient way for detailled avalanche logging in files.
	If this is not wanted, array_stack is faster.
	@invariant write_ptr always points to the element last written
*/
template<class T>
class _array_queue : public _array_queue_base<T>, public log_base<T>
{
	typedef _array_queue_base<T> base;
public:
	inline _array_queue(unsigned human_grid_size, FILE* fp) :
		base(human_grid_size),
		log_base<T>(fp) {}

	// logging:
	inline void write_to_file() const {
		log_base<T>::write_array_to_file(
			base::array+1, base::write_ptr - base::array);
	}
};

typedef _array_queue<int*> array_queue;

template<class T>
class _array_queue_no_file : public _array_queue_base<T>, public log_nothing_base<T>
{
	typedef _array_queue_base<T> base;
public:
	inline _array_queue_no_file(unsigned human_grid_size) :
		base(human_grid_size) {}

	// logging:
	inline void write_to_file() const {}
};

typedef _array_queue_no_file<int*> array_queue_no_file;

/*
 * algorithms
 */
namespace internal
{

/* This recursive function is slower, so unused
template<class T>
void rec(const signed& grid_width, T const ptr)
{
	T const ptr_e = ptr + 1;
	if(++*ptr_e > 3) {
		*ptr_e&=3;
		rec(grid_width, ptr_e);
	}
	T const ptr_w = ptr - 1;
	if(++*ptr_w > 3) {
		*ptr_w&=3;
		rec(grid_width, ptr_w);
	}
	T const ptr_s = ptr + grid_width;
	if(++*ptr_s > 3) {
		*ptr_s&=3; // TODO: template variant with ==4 => = 0 ?
		rec(grid_width, ptr_s);
	}

	T const ptr_n = ptr - grid_width;
	if(++*ptr_n > 3) {
		*ptr_n&=3;
		rec(grid_width, ptr_n);
	}
}*/

/**
	Develops an 1D avalanche. The helping avalanche container is not flushed, so it contains the whole avalanche afterwards.
	Important: The cell at hint must be decreased by 1.
*/
template<class AvalancheContainer>
// TODO: make hint ptr -> no grid class
// TODO: use refs for ints?
inline void avalanche_1d_hint_noflush(const signed& grid_width, AvalancheContainer& array,
	typename AvalancheContainer::value_type hint) // TODO: hint is a const pointer!
{
	*hint -= 4; // can be <0, so "*hint & 3" is not correct here
	array.push(hint);
	do
	{
		using vt = typename AvalancheContainer::value_type;
		vt const ptr = array.pop();

		vt const ptr_e = ptr + 1;
		if(++*ptr_e > 3) {
			*ptr_e&=3;
			array.push(ptr_e);
		}
		vt const ptr_w = ptr - 1;
		if(++*ptr_w > 3) {
			*ptr_w&=3;
			array.push(ptr_w);
		}
		vt const ptr_s = ptr + grid_width;
		if(++*ptr_s > 3) {
			*ptr_s&=3; // TODO: template variant with ==4 => = 0 ?
			array.push(ptr_s);
		}

		vt const ptr_n = ptr - grid_width;
		if(++*ptr_n > 3) {
			*ptr_n&=3;
			array.push(ptr_n);
		}

	} while( ! array.empty() );
	array.write_to_file();
}

//! variant with @dim parameter instead of giving width as int
template<class T, class AvalancheContainer>
inline void avalanche_1d_hint_noflush(std::vector<T>& grid, const dimension& dim, const uint32_t hint, AvalancheContainer& array)
{
	internal::avalanche_1d_hint_noflush(dim.width(), array, &grid[hint]);
}

} // namespace internal

template<class T, class AvalancheContainer>
inline void avalanche_1d_hint_noflush_single(std::vector<T>& grid, const dimension& dim, const uint32_t hint, AvalancheContainer& array)
{
	array.write_header((uint64_t)grid.data());
	internal::avalanche_1d_hint_noflush(dim.width(), array, &grid[hint]);
}

template<class AvalancheContainer>
inline void avalanche_1d_hint_noflush_single(grid_t& grid, const point& hint, AvalancheContainer& array)
{
	array.write_header((uint64_t)grid.data().data());
	internal::avalanche_1d_hint_noflush(grid.internal_dim().width(), array, &grid[hint]);
}

/**
	Develops an 1D avalanche. The helping avalanche container is flushed.
	Important: The cell at hint must be decreased by 1.
*/
template<class T, class AvalancheContainer>
inline void avalanche_1d_hint(std::vector<T>& grid, const dimension& dim, const uint32_t hint, AvalancheContainer& array)
{
	internal::avalanche_1d_hint_noflush(grid, dim, hint, array);
	array.flush(); // note: empty does not always imply being flushed!
}

/**
	Develops an xD avalanche for an x>=0. Writes avalanche seperator afterwards.
	@param times Number of times that the cell at hint may fire. for times=INT_MAX, lx_hint = l_hint
*/
template<class T, class AvalancheContainer>
inline void lx_hint(std::vector<T>& grid, const dimension& dim, const uint32_t hint, AvalancheContainer& array, int times)
{
	array.write_header((uint64_t)grid.data());
	grid[hint]--;
	for(;grid[hint]>2 && times > 0; times--)
	{
		avalanche_1d_hint(grid, dim, hint, array);
	}
	array.write_separator();
	grid[hint]++;
}

/**
	Develops a full avalanche and writes avalanche seperator afterwards.
	@param array container of type array_stack or array_queue.
		array_stack is faster (1-2 times), but array_queue can handle IO (instantly!).
*/
template<class T, class AvalancheContainer>
inline void l_hint(std::vector<T>& grid, const dimension& dim, const uint32_t hint, AvalancheContainer& array)
{
	array.write_header((uint64_t)grid.data());
	grid[hint]--;
	while(grid[hint]>2)
	{ // TODO: fit this for char types: all 128 rounds (lx_hint?)
		avalanche_1d_hint(grid, dim, hint, array);
	//	avalanche_1d_hint_noflush_2<int>(grid, dim, hint/*, array, avalanche_fp*/);
	}
	array.write_separator();
	grid[hint]++;
}

#if 0
template<class AvalancheContainer>
inline void l2_hint(std::vector<int>* grid, const dimension* dim, int hint, AvalancheContainer* array, FILE* avalanche_fp)
{
	grid[hint]--;
	while(grid[hint]>2)
	{
		avalanche_1d_hint2(grid, dim, hint, array, avalanche_fp);
	}
	array.write_separator(avalanche_fp);
	grid[hint]++;
}
#endif

template<class T>
using _fix_log_l = log_base<T>;
template<class T>
using _fix_log_s = log_nothing_base<T>;
using fix_log_l = _fix_log_l<int*>;
using fix_log_s = _fix_log_s<int*>;

/*
 * fix algorithms
 */
//! TODO: fix should be able to substract the superstable id
template<class AvalancheContainer, class ResultType>
inline void do_fix(/*std::vector<int>* grid,*/ const dimension& dim, AvalancheContainer& array, ResultType& result_logger)
{
	uint32_t fire_times;
	const int INVERT_BIT = (1 << 31);
	const int GRAIN_BITS = (-1) ^ INVERT_BIT;

//	result_logger.write_avalanche_counter();

	do
	{
		using vt = typename AvalancheContainer::value_type;
		vt const cur_element = array.pop(); // TODO!!
		//printf("cur: %d\n",cur_element);

		*cur_element &= GRAIN_BITS;
		fire_times = *cur_element >>2; // keep up invariant: elements in array are already decreased
		*cur_element -= (fire_times<<2);

		result_logger.write_elem_to_file(cur_element, &fire_times);

		vt const e = cur_element + 1;
		if((*e+=fire_times) > 3) {
			*e |= INVERT_BIT;
			array.push(e);
		}
		vt const w = cur_element-1;
		if((*w+=fire_times) > 3) {
			*w |= INVERT_BIT;
			array.push(w);
		}
		vt const s = cur_element + dim.width();
		if((*s+=fire_times) > 3) {
			*s |= INVERT_BIT;
			array.push(s);
		}
		vt const n = cur_element - dim.width();
		if((*n+=fire_times) > 3) {
			*n |= INVERT_BIT;
			array.push(n);
		}

	} while( ! array.empty() );
}

/**
	This is the alternative algorithm for sandpiles with many more than 3 grains.
	Proposed by Sebastian Frehmel in his Diploma Thesis.
	@param result_logger Class of type FixLogL or FixLogS
*/
template<class AvalancheContainer, class ResultType>
inline void fix(std::vector<int>& grid, const dimension& dim, int hint, AvalancheContainer& array, ResultType& result_logger)
{
	//printf("hint: %d\n",hint);
	result_logger.write_header((uint64_t)grid.data());
	if(grid[hint]>3)
	{
		array.push(&(grid[hint]));
		do_fix(dim, array, result_logger);
	}
	result_logger.write_separator();
	array.flush(); // note: empty does not always imply being flushed!
}

// TODO: remove non-grid_t-versions everywhere

//! version without a hint
template<class AvalancheContainer, class ResultType>
inline void fix(std::vector<int>& grid, const dimension& dim, AvalancheContainer& array, ResultType& result_logger)
{
	const int INVERT_BIT = (1 << 31);

	result_logger.write_header((uint64_t)grid.data());
	for(unsigned int count = 0; count < dim.area(); ++count)
	{
		if(! is_border(dim, count)) {
			// TODO: improve this!
			array.push(&(grid[count])); // panic: every cell is assumed to be higher than 3
			grid[count] |= INVERT_BIT; // don't push this one twice
		}
	}

	// note: hint does not matter for correctness
	do_fix(dim, array, result_logger);
}

}

#endif // STACK_ALGORITHM_H
