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

// io stuff, not interesting...

#include <cstring>
#include <set>
#include <iostream>

#include "general.h"

#include "base.h"
#include "odometer.h" // TODO: io routine?
#include "util.h"

#include "io/secfile.h"

namespace input_file {

using namespace sca::io;

using m_map = std::map<std::size_t, std::vector<std::set<types::point>>>;

class initial_t : public types
{
public:
	std::vector<std::vector<conf_t>> confs;
	m_map* ini_cells;
	grid_t* grid;

	initial_t(m_map* ini_cells) : // TODO: init grid_t here, too?
		ini_cells(ini_cells) {}

	void read(std::istream& stream)
	{
		std::cerr << "INICELLS:" << mk_print(*ini_cells);
		confs.resize(ini_cells->size());

		char buf[1024];

		const auto next = [&](std::istream& stream) -> std::size_t {
			stream.getline(buf, sizeof(buf));
			return (!strcmp(buf, "end")) ? 0 : atoi(buf);
		};

		std::size_t grid_id;

		backed_up_grid bug(*grid);

		while((grid_id = next(stream)))
		{
			--grid_id; // this fixes the human id-ing
			stream.getline(buf, sizeof(buf)); // empty line
			if(*buf)
			 throw "expected newline";
			std::vector<std::set<point>>& l = ini_cells->at(grid_id);

			std::cerr << "Number of initial confs: " << l.size() << std::endl;

			std::set<point> whole;

			for(std::size_t i = 0; i < l.size(); ++i)
			{
				grid_t conf_grid(std::cin, 0);
				std::cerr << " - reading conf: " << conf_grid  << ", sub area " << i << std::endl;
				conf_t tmp_conf(conf_grid.internal_dim(), conf_grid);

				std::cerr << "   -> initial cell: " << mk_print(l[i]) << std::endl;

				_reset_grid_to_conf(bug, tmp_conf, l[i]);
				for(const point& p : l[i])
				 whole.insert(p);
			}

			confs[grid_id].emplace_back(whole, *grid);
		}
	}

	friend std::istream& operator>>(std::istream& stream,
		initial_t& ini) {
		return ini.read(stream), stream;
	}

	friend std::ostream& operator<<(std::ostream& stream, const initial_t& ) {
		return stream;
	}
};

class unite_t : public supersection_t, types
{
public:
	m_map* ini_cells;

	unite_t(m_map* ini_cells) :
		supersection_t(type_t::multi),
		ini_cells(ini_cells) {
		// (TODO: delayed_leaf) ?
		init_factory<leaf_template_t<types::grid_t>>();
	}

	void process()
	{
		for(const auto& pr : numbered_values<types::grid_t>())
		{
			std::size_t grid_no = pr.key();
			const types::grid_t& ids = pr.value();

			ini_cells->emplace(grid_no-1, std::vector<std::set<point>>());
			for(const types::grid_t::value_type& c : ids)
			{
#ifdef VERBOSE_OUTPUT
				std::cout << grid_no << "<->" << c << std::endl;
#endif
				//ini_cells.at(grid_no-1).unite(ini_cells.at(c-1));
				auto& me = ini_cells->at(grid_no-1);
				const auto& other = ini_cells->find(c-1); // (TODO: std::move?)

				assert_always(other->second.size() == 1,
					"other.points.size()");
				me.push_back(other->second.front());

				/* this probably won't work:
				me.push_back(std::move(other->second.front()));
				ini_cells->erase(other);*/
			}
		}
	}
};


class input_file_t : public supersection_t, types // TODO: public?
{
public:
	using ini_pair = std::pair<std::size_t, std::vector<types::point>>;
	m_map ini_cells;
	int border_width, dead_state;

	void on_grid() // (TODO: pass ptr here?)
	{
		types::grid_t& grid = value<types::grid_t>("grid");

		for(const types::point& p : grid.internal_points()) {
			point _p = p - point(border_width, border_width);
			if(!grid.contains(_p))
			 grid.at_internal(p) = dead_state;
		}
		grid.print_with_border(std::cerr);

		for( const point& p : grid.points() )
		{
			const types::cell_t val = grid[p];
			if(val < 0)
			{
				const types::cell_t internal_pos = -val - 1;

				auto& v = ini_cells.emplace(internal_pos,
					std::vector<std::set<point>>()).
					first->second;

				if(v.size() == 0)
				 v.resize(1);
				assert(v.size() == 1);
				v.front().insert(p);
			}
		}

		value<initial_t>("initial").grid = &grid;
	}

	input_file_t(int bw, int dead_state) : supersection_t(type_t::sections),
		border_width(bw << 1),
		dead_state(dead_state)
	{
		init_leaf<leaf_template_t<std::string>>("name");
		init_leaf<leaf_template_t<std::string>>("rgb32");

		// we are a bit over-careful with the border width...
		init_leaf_cb<leaf_template_t<types::grid_t>>("grid", (hook_func_t)&input_file_t::on_grid, border_width);
		init_subsection<unite_t>("unite", &ini_cells);
		init_leaf<leaf_template_t<initial_t>>("initial", &ini_cells);
	}
};

}

void base::_parse()
{
	std::cerr << std::endl << "IO starting..." << std::endl << std::endl;

	sca::io::secfile_t inf;
	input_file::input_file_t input(ca.border_width(), dead_state);

	try {
		inf >> input;
	} catch(io::secfile_t::error_t ife) {
		std::cerr << "infile line " << ife.line << ": "	 << ife.msg << std::endl;
		throw "infile parse error";
	}

// init pseudo root first
/*#ifdef USE_GRAPH
#ifdef USE_LABELS
	get(pseudo_root).label = "pseudo_root";
#endif
#endif*/
//	graph[pseudo_root].node_type = node_type_t::extend;

	for(const std::pair<std::size_t, std::vector<std::set<point>>>& pr : input.ini_cells)
	for(const std::set<point>& s : pr.second)
	for(const point& p : s)
	{
		initial_area_all.insert(p);
	}


	sim_grid = orig_grid = orig_grid_unchanged
		= std::move(input.value<grid_t>("grid"));

	const auto& confs = input.value<input_file::initial_t>("initial").confs;

	auto cb = [&](const std::vector<std::vector<conf_t>::const_iterator>& iv)
	{
		std::size_t cell = 0;
		for(const auto itr : iv)
		{
			if(confs[cell].size())
			{
				std::set<point> whole;
				for(std::size_t i = 0; i < input.ini_cells[cell].size(); ++i)
				for(const point& p : input.ini_cells[cell][i])
				 whole.insert(p);
				_reset_grid_to_conf(sim_grid, *itr, whole);
			}
			++cell;
		}

		std::cerr << "initial grid read: " << std::endl << sim_grid << std::endl;
		initial_confs.push_back(conf_t(initial_area_all, sim_grid));
	};

	for(const point& p : ca_n(initial_area_all))
	if(ca.is_cell_active(sim_grid, p))
	 all_points.insert(p);

	// TODO: inherited odometer which gives elems instead of itrs?
	odometer<true>(confs, cb);

	// meta
	name = std::move(input.value<std::string>("name"));
	rgb32 = std::move(input.value<std::string>("rgb32"));
}



