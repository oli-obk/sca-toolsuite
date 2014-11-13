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

#include <ostream>
#include "results.h"
#include "print.h"

std::ostream &operator<<(std::ostream &stream, const rec_pair &r)
{
	return stream << "rec_pair( points: " << mk_print(r.points)
		<< ", result: " << r.result
		<< ", scc size: "
		<< rec_pair::get_exactness_string(r.exactness)
		<< " " << r.scc_size << ")";
}

std::ostream &operator<<(std::ostream &stream, const rec_rval_base_t &rval)
{
	return stream << "rec_rval_base_t(" << std::endl
		<< "  final candidates: " << mk_print(rval.final_candidates)
		<< ", " << std::endl
		<< "  total area: " << mk_print(rval.total_area) << ")"
		<< std::endl;
}

void rec_pair::print_pretty(types::grid_t &tmp_grid, std::ostream &stream)
	const
{
#if 0
	result.apply_fwd(tmp_grid);

	stream << "patch: "<< std::endl << result

		<< std::endl
		<< "full grid: " << std::endl
		<< tmp_grid

		<< "scc_size: " << rec_pair::get_exactness_string(exactness)
		<< " " << scc_size << std::endl << std::endl;

	result.apply_bwd(tmp_grid);
#else
	(void)tmp_grid;
	(void)stream;
	throw "Sorry, currently not implemented.";
#endif
}

void rec_rval_base_t::print_results_pretty(const grid_t& orig_grid) const
{
	grid_t res_g = orig_grid;
	for(const rec_pair& rp : final_candidates)
	{
		rp.print_pretty(res_g);
	}
}

//std::size_t result_t::next_id = 0;

