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

#ifndef RANDOM_H
#define RANDOM_H

#include <sys/types.h>
#include <unistd.h>
#include <ctime>

// TODO: do not inline the first two functions -> cpp file
// TODO: namespace

//! Gets a seed which should differ much enough from time to time.
inline unsigned int find_good_random_seed(void) {
	return (unsigned int)time(NULL) + getpid() + getppid() + 42;
}
//! Calls function of OS to set a randome seed value.
inline void set_random_seed(unsigned int seed) {
	srandom(seed);
}
//! Calls function of OS to set a randome seed value.
inline void set_random_seed() {
	srandom(find_good_random_seed());
}
//! Returns a random value out of max subsequent values, beginning with 0
//! In other words, the result will be in [0, max-1]
inline unsigned int get_random_int(unsigned int max) {
	return (unsigned int) (((float)max)*random()/(RAND_MAX+1.0));
}

#endif // RANDOM_H
