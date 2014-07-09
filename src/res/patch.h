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

#ifndef PATCH_H
#define PATCH_H

#include <set>

#include "grid.h" // TODO: needed?
#include "ca.h"

namespace sca
{

template<bool ExtendedFormat, class Traits, class CellTraits>
class _patch_t
{
	using point = _point<Traits>;
	using conf_t = ca::_conf_t<CellTraits>;
	using grid_t = _grid_t<Traits, CellTraits>;
	using cell_t = typename CellTraits::cell_t;


	std::set<point> _area;
	conf_t _conf;
	conf_t _conf_before;

	_patch_t add__patch_t(const std::set<point>& rhs_area,
		const conf_t& rhs_old,
		const conf_t& rhs_new
		) const
	{
		_patch_t res;

		using set_itr = typename std::set<point>::const_iterator;
		using ca::true_func;

		// initialize
		std::size_t res_size = 0;

		auto cb_init_oneonly = [&](const ca::counted_itr<set_itr>& itr) {
			++res_size;
			res._area.insert(res._area.end(), *itr);
		};

		auto cb_init_both = [&](const ca::counted_itr<set_itr>& itr1, const ca::counted_itr<set_itr>& itr2) {
			if(_conf[itr1->id()] != rhs_old[itr2->id()])
			{
				std::cout << "Calculating " << *this
					<< " + (" << mk_print(rhs_area)
					<< ", " << rhs_old << "-> " << rhs_new << ")..." << std::endl;
				assert_always(_conf[itr1->id()] == rhs_old[itr2->id()], "Confs can not be added");
			}
			const bool can_optimize = (_conf_before[itr1->id()] == rhs_new[itr2->id()]);
			if(!can_optimize)
			{
				++res_size;
				res._area.insert(res._area.end(), *itr1);
			}
		};

		ca::zip(ca::counted(_area), ca::counted(rhs_area), ca::zipper(
			true_func(cb_init_oneonly),
			true_func(cb_init_oneonly),
			true_func(cb_init_both)
			));

		res._conf_before.data().resize(res_size);
		res._conf.data().resize(res_size);

		// zip confs
		typename std::vector<cell_t>::iterator itr_old = res._conf_before.begin(),
			itr_new = res._conf.begin();
		auto only_this = [&](const ca::counted_itr<set_itr>& itr) {
			*(itr_old++) = _conf_before[itr->id()];
			*(itr_new++) = _conf[itr->id()];
		};
		auto only_rhs = [&](const ca::counted_itr<set_itr>& itr) {
			*(itr_old++) = rhs_old[itr->id()];
			*(itr_new++) = rhs_new[itr->id()];
		};
		auto both = [&](const ca::counted_itr<set_itr>& itr1, const ca::counted_itr<set_itr>& itr2) {
			const bool can_optimize = (_conf_before[itr1->id()] == rhs_new[itr2->id()]);
			if(!can_optimize)
			{
				*(itr_old++) = _conf_before[itr1->id()];
				*(itr_new++) = rhs_new[itr2->id()];
			}
		};
		ca::zip(ca::counted(_area), ca::counted(rhs_area), ca::zipper(
			true_func(only_this),
			true_func(only_rhs),
			true_func(both)
			));

		assert(res._conf.size() == res._area.size());
		return res;
	}


	void apply_single(grid_t& current, const ca::counted_itr<typename std::set<point>::const_iterator>& p) const
	{
		if(current[*p] != _conf_before[p->id()])
		{
			std::cout << "Error applying: " << *this << " on "<< current << std::endl;
		}
		assert_always(current[*p] == _conf_before[p->id()],
			"This patch can not be applied on this grid.");
		current[*p] = _conf[p->id()];
	}

public:

	_patch_t() {}
	_patch_t(const std::set<point>& area, const conf_t& conf, const conf_t& old_conf)
	{
		std::size_t reserve_size = 0;
		for(const auto& elem : ca::counted(area))
		{
			if(conf[elem.id()]!=old_conf[elem.id()])
			{
				++reserve_size;
			}
		}

		_conf.data().reserve(reserve_size);
		_conf_before.data().reserve(reserve_size);

		for(const auto& elem : ca::counted(area))
		{
			if(conf[elem.id()]!=old_conf[elem.id()])
			{
				_area.insert(elem);
				_conf.data().push_back(conf[elem.id()]);
				_conf_before.data().push_back(old_conf[elem.id()]);
			}
		}
	}
	_patch_t(const point& p, const cell_t& cell_new, const cell_t& cell_old) :
		_area((cell_new == cell_old)?std::set<point>{}:std::set<point>{p}),
		_conf((cell_new == cell_old)?conf_t{}:conf_t(cell_new)),
		_conf_before((cell_new == cell_old)?conf_t{}:conf_t(cell_old))
	{
	}

	//! @param g1 new grid
	//! @param g2 old grid
	_patch_t(const grid_t& g1, const grid_t& g2)
	{
		assert_always(g1.size() == g2.size(), "grids have different sizes");
		// TODO: unify with the above code
		std::size_t reserve_size = 0;
		// TODO: better: double itr over g1 and g2
		for(const auto& elem : g1.points())
		{
			if(g1[elem]!=g2[elem])
			{
				++reserve_size;
			}
		}

		_conf.data().reserve(reserve_size);
		_conf_before.data().reserve(reserve_size);

		for(const auto& elem : g1.points())
		{
			if(g1[elem]!=g2[elem])
			{
				_area.insert(elem);
				_conf.data().push_back(g1[elem]);
				_conf_before.data().push_back(g2[elem]);
			}
		}
	}

	friend std::ostream& operator<< (std::ostream& stream,
		const _patch_t& c)
	{
		if(ExtendedFormat)
		{
			_bounding_box<Traits> bb;
			for(const point& p : c._area)
			 bb.add_point(p); // TODO: bb ctor from container
			_rect<Traits> rc(bb.ul(), bb.lr());

			point last_point = *rc.begin();

			using set_itr = typename std::set<point>::const_iterator;
			using ca::true_func;

			auto cb_left = [](const ca::counted_itr<set_itr>&) {};
			auto cb_right = [&](const ca::counted_itr<point_itr>& itr) {
				if(((point)*itr).y - last_point.y) // TODO...
				 stream << std::endl;
				stream << "  ";
				last_point = (point)*itr;
			};

			auto cb_both = [&](const ca::counted_itr<set_itr>& itr, const ca::counted_itr<point_itr>&)
			{
				if(((point)*itr).y - last_point.y) // TODO...
				 stream << std::endl;
				//if(c._area.find(*itr) == c._area.end())
				// stream << " X";
			//	else
				 stream << (int)c._conf[itr->id()] << " ";
				last_point = *itr;
			};

			stream << "_patch_t: " << rc << ", " << std::endl;
			ca::zip(ca::counted(c._area), ca::counted(rc), ca::zipper(
				true_func(cb_left),
				true_func(cb_right),
				true_func(cb_both)
			));
			return stream;
		}
		else return (c._area.empty())
			? (stream << "patch( empty )")
			: (stream << "patch( " << mk_print(c._area) << ", "
				<< c._conf_before << " -> "
				<< c._conf << ")");
	}

	bool operator<(const _patch_t& rhs) const
	{
		if(rhs._area == _area)
		{
			if(_conf == rhs._conf)
			 return _conf_before < rhs._conf_before;
			else
			 return _conf < rhs._conf;
		}
		else
		 return _area < rhs._area;
	}

	// TODO: common function using a template
	void apply_fwd(grid_t& current) const
	{
		for(const auto& p : ca::counted(_area))
		{
			if(current[p] != _conf_before[p.id()])
			{
				std::cout << "Error applying: " << *this << " on "<< current << std::endl;
			}
			if(current[p] != _conf_before[p.id()])
				throw "This patch can not be applied on this grid.";
			current[p] = _conf[p.id()];
		}
	}

	void apply_fwd(grid_t& current, const std::set<point>& selection) const
	{
		using set_itr = typename std::set<point>::const_iterator;
		using ca::true_func;

		const auto cb_apply_single = [&](const ca::counted_itr<set_itr>& p) {
			apply_single(current, p);
		};

		ca::zip(ca::counted(_area), ca::counted(selection),
			ca::zipper(true_func(cb_apply_single))
		);
	}


	bool can_apply_fwd(const grid_t& current) const
	{
		bool answer = true;
		for(const auto& p : ca::counted(_area)) // TODO: allow break for iterator
		{
			answer = answer && (current[p] == _conf_before[p.id()]);
			if(!answer)
			 break;
		}
		return answer;
	}

	void apply_bwd(grid_t& current) const
	{
		for(const auto& p : ca::counted(_area))
		{
			assert_always(current[p] == _conf[p.id()],
				"This patch can not be applied on this grid.");
			current[p] = _conf_before[p.id()];
		}
	}

	_patch_t operator+(const _patch_t& rhs) const
	{
		return add__patch_t(rhs._area, rhs._conf_before, rhs._conf);
	}

	_patch_t operator-(const _patch_t& rhs) const
	{
		return add__patch_t(rhs._area, rhs._conf, rhs._conf_before);
	}

	_patch_t operator-() const
	{
		return _patch_t(std::set<point>(_area), conf_t(_conf_before), conf_t(_conf));
	}

	//! this will let us behave as executed, and then other executed
	const _patch_t& operator+=(const _patch_t& rhs)
	{
		return (*this = *this + rhs);
	}

	//! this will let us behave as executed, and then other reverse executed
	const _patch_t& operator-=(const _patch_t& rhs)
	{
		return (*this = *this - rhs);
	}

	//! nothing special
	bool operator==(const _patch_t& rhs) const
	{
		return (_area == rhs._area)
			&& (_conf == rhs._conf)
			&& (_conf_before == rhs._conf_before);
	}

	const std::set<point>& area() const { return _area; }
	const conf_t& conf() const { return _conf; }
};

}

#endif // PATCH_H
