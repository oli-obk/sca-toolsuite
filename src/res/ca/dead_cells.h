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

#ifndef DEAD_CELLS_H
#define DEAD_CELLS_H

#include "ca.h"
#include "grid.h"

namespace sca { namespace ca {

template<class Solver, class Traits, class CellTraits>
class dead_cell_scan
{
	using calculator_t = _calculator_t<Solver, Traits, CellTraits>;
	const calculator_t& calc;

	using cell_t = typename CellTraits::cell_t;
	using u_coord_t = typename Traits::u_coord_t;

	const cell_t num_states;

	const n_t &n_in, &n_out, n_in_inv, n_out_inv;

public:

	enum states
	{
		cell_default = 0,
		cell_passive = 1,
		cell_dead = 2,
		cell_dead_state = 3
	};

	dead_cell_scan(const calculator_t& calc, const cell_t& num_states) :
		calc(calc),
		num_states(num_states),
		n_in(calc.n_in()),
		n_out(calc.n_out())
	{
	}

	// dead: the cell can not change in no way
	// passive: the cell may change by activation of others,
	// however, there is no state where it gets active
	// active: the cell itself may get active

	// TODO: res as boolean grid?
	grid_t operator()(const grid_t& src)
	{
		if(src.border_width() < calc.border_width()) {
			throw "Input grid must have border width"
				"greater-equal than ca.";
		}

		// step 1: mark outer border dead states
		grid_t dead(src.human_dim(), calc.border_width(),
			cell_default, cell_dead_state);

		const u_coord_t bw = calc.border_width();
		const point max = point(src.human_dim().dx() - bw, src.human_dim().dy() - bw);
		rect inner_rect(point(bw, bw), max);

		std::vector<point> killed[2];

		// step 2: mark dead states + inner border dead states
		for(const point& p : src.human_dim())
		{
			const auto mark_killed = [&](const point& p) {
				killed[1].push_back(p);
			};
			if(calc.is_state_dead(src[p]))
			{
				dead[p] = cell_dead_state;
				mark_killed(p);
			}
			else if(!inner_rect.contains(p)) {
				dead[p] = cell_dead;
				mark_killed(p);
			}
		}


		// step 3: mark passive
		grid_t tmp_grid(n_in.get_dim(), 0);
		point tmp_center = n_in.get_center_cell();

		const auto check_passive = [&](const point& p)
		{
			// TODO: grids should allow negative values for indices

			tmp_grid.reset(0);

			std::vector<point> not_dead;

			for(const point& np : n_in)
			if(dead[p+np] >= cell_dead) {
				tmp_grid[tmp_center + np] = src[p+np];
			}
			else
			 not_dead.push_back(tmp_center + np);

			const auto& is_not_active = [&](const grid_t& _tmp_grid){
				return !calc.is_cell_active(_tmp_grid, tmp_center);
			};

			return iterate_grid_bool(tmp_grid, not_dead, num_states,
				is_not_active);
		};

		int round = 0;

		do
		{
			++round;
			std::set<point> candidates = n_in(killed[round & 1]); // TODO: inverse!

			killed[round & 1].clear();

			for(const point& p : candidates)
			{
				if(dead[p] < cell_passive && check_passive(p))
				{
					dead[p] = cell_passive;
					killed[(round + 1) & 1].push_back(p);
				}
			}

		} while(killed[(round + 1) & 1].size()) ;

		return dead;

	}
};

}}

#endif // DEAD_CELLS_H
