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

#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

#include "brute_config.h"

/*
 * handle disabling boost graph
 */
#ifdef NO_BOOST_GRAPH
#ifdef DEBUG_GRAPH
	#warning "Debug graph turned off due to -DNO_DEBUG_GRAPH."
	#undef DEBUG_GRAPH
#endif
#else
	#define EN_BOOST_GRAPH
#endif

/*
 * macros for debug graph - CAREFULLY change!
 */
#define DELETE_UNUSED_VERTS //!< whether to delete DFS-finished verts
//#define REMOVE_ORPHANS //!< whether to remove paths with unused results

#endif // COMMON_MACROS_H
