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

#include "general.h"
#include "print.h"
#include "utils/name_type_map.h"
#include "types.h"
#include "results.h"
#include "stats.h"
#include "disjoint_sets.h"
#include "common_macros.h"

#ifdef EN_BOOST_GRAPH
 #include "old_dep_graph.h"
#endif
// TODO: rename: evaluate?

std::string stl_style_name(const std::string& src) {
	std::string res;
	res.reserve(src.size());
	for(const char& c : src)
	{
		char c2 = std::tolower(c);
		if(c2 == ' ')
		 c2 = '_';
		res.push_back(c2);
	}
	return res;
}

enum class mode
{
	help,
	diffs,
	depgraph,
	depgraph_dump,
	paths,
	paths_dump,
	used_points,
	tex,
	invalid
};

constexpr std::size_t mode_sz =
#ifdef EN_BOOST_GRAPH
	8;
#else
	4;
#endif

util::name_type_map_t<mode_sz, mode, mode::invalid> modes =
{{
	{ "help", mode::help },
	{ "diffs", mode::diffs },
#ifdef EN_BOOST_GRAPH
	{ "depgraph", mode::depgraph },
	{ "depgraph_dump", mode::depgraph_dump },
	{ "paths", mode::paths },
	{ "paths_dump", mode::paths_dump },
#endif
	{ "activated", mode::used_points },
	{ "tex", mode::tex }
}};

struct data : public types
{
	std::string name, rgb32;
	grid_t orig_grid;
#ifdef EN_BOOST_GRAPH
	using m_dep_graph_t = dep_graph_t<point>;
	m_dep_graph_t dep_graph;
#endif
	rec_rval_base_t results;
	std::vector<point> used_points;
	grid_t activated;
	m_graph_t<result_t> res_graph;

	stat_results_t stats;

	data()
#ifdef EN_BOOST_GRAPH
		: dep_graph("depgraph", false)
#endif
	{}

	friend io::deserializer& operator>>(io::deserializer& s, data& d)
	{
		int32_t version;
		s >> version;
		if(version != 1)
		 throw "Incompatible versions.";
		s >> d.name >> d.rgb32;
		s >> d.orig_grid;
		s >> d.results;
		std::size_t num_edges;
		s >> num_edges;
		/*s >> dep_graph.num_edges();
		for(const dep_graph::edge_t e : dep_graph.edges())
		{
			s << dep_graph.get(dep_graph.source(e)).p
				<< dep_graph.get(dep_graph.target(e)).p;
		}*/

		d.activated = grid_t(d.orig_grid.human_dim(), 0, 0);

#ifdef EN_BOOST_GRAPH
		std::map<point, m_dep_graph_t::vertex_t> vertex_of;
#endif
		for(std::size_t i = 0; i < num_edges; ++i)
		{
			point p1, p2;
			s >> p1 >> p2;
#ifdef EN_BOOST_GRAPH
			auto itr1 = vertex_of.find(p1);
			if(itr1 == vertex_of.end())
			// TODO: use insert hint
			 vertex_of[p1] = d.dep_graph.add_vertex(p1);
			auto itr2 = vertex_of.find(p2);
			if(itr2 == vertex_of.end())
			// TODO: use insert hint
			 vertex_of[p2] = d.dep_graph.add_vertex(p2);
			d.dep_graph.add_edge(vertex_of[p1], vertex_of[p2]);
#endif
			d.activated[p1] = 1;
			d.activated[p2] = 1;
		}

		s >> d.res_graph;

		s >> d.stats;

		//std::cerr << d.res_graph;

		return s;
	}

	void dump_as_tex()
	{
		enum class io_type
		{
			in = 1,
			out = 2,
			in_out = 3
		};

		grid_t comp_grid(orig_grid.human_dim(), 0, 0);
		std::set<point> search;

//		std::cerr << "ORIG GRID: " << orig_grid << std::endl;
		for(const point& p : orig_grid.points())
		{
			const cell_t& val = orig_grid[p];
			if(val < 0) {
				comp_grid[p] |= (std::size_t)io_type::in;
				search.insert(p);
			}
		}

//		std::cerr << "COMP GRID: " << comp_grid << std::endl;
		for(const rec_pair& r : results.final_candidates)
		{
			for(const point& p : r.result.area())
			{
				comp_grid[p] |=  (std::size_t)io_type::out;
				search.insert(p);
			}
		}

		const auto cb_abort = [&](const point& _old, const point& _new) {
			// this aborts for any 2<->1/3
			return (!comp_grid[_new] || (comp_grid[_old] != comp_grid[_new] && (comp_grid[_old] | comp_grid[_new]) == 3) );
		};
		grid_scc_finder_t<grid_t> scc_finder(orig_grid.human_dim());
		std::vector<std::vector<point>> all_areas =
			scc_finder(n_t::neumann_1_2d(), search, cb_abort);

		enum class io_idx
		{
			in = 0,
			out = 1,
			in_out = 2,
			size = 3
		};

		std::vector<std::vector<point>> areas[(std::size_t)io_idx::size];

//		std::cerr << "COMP GRID: " << comp_grid << std::endl;
		for(std::vector<point>& v : all_areas)
		{
			int type = 0;
			for(const point& p : v)
			 type |= comp_grid[p];
			areas[type-1].push_back(std::move(v));
		}

		// evaluate graph, fill in/out cells

		// start a dfs
		using vertex_t = std::pair<const result_t*, bool>;
		std::stack<vertex_t> stack;

		std::vector<grid_t> io_grids[(std::size_t)io_idx::size];
		std::vector<rect> rects[(std::size_t)io_idx::size];

		const auto cb_init = [&](const io_idx& idx)
		{
			for(const std::vector<point> v : areas[(std::size_t)idx])
			{
				bounding_box bb;
				for(const point& p : v)
				 bb.add_point(p);
				rects[(std::size_t)idx].push_back(bb.rect());
				io_grids[(std::size_t)idx].emplace_back(bb.dim(), 0, 0);
			}
		};

		cb_init(io_idx::in);
		cb_init(io_idx::out);
		cb_init(io_idx::in_out);

		std::size_t row_size = io_grids[(std::size_t)io_idx::in].size()
			+ io_grids[(std::size_t)io_idx::out].size()
			+ (io_grids[(std::size_t)io_idx::in_out].size() << 1)
			;


		std::ostream& o = std::cout;
		std::size_t grid_counter = 0; // first grid will be fixed

		std::vector<std::string> scc_sizes;

		// caption grids
		const auto cb_caption = [&](const io_idx& idx)
		{
			std::size_t i = (std::size_t)idx;
			for(std::size_t j = 0; j < io_grids[i].size(); ++j)
			{
				cell_t area_id = scc_finder.visited_grid()[areas[i][j].front()];
				for(cell_t& c : io_grids[i][j])
				 c = -area_id;
				o << grid_counter++ << "\n\n" << io_grids[i][j] << "\n";
			}
		};


		const auto _fill = [&](const io_idx& idx)
		{
			std::size_t i = (std::size_t)idx;
			for(std::size_t j = 0; j < areas[i].size(); ++j)
			{
				int bw = orig_grid.border_width();
				orig_grid.copy_to_int(io_grids[i][j],
					rects[i][j] + point(bw, bw));
					std::cerr << "RES GRID: \n" << io_grids[i][j] << std::endl;
				o << grid_counter++ << "\n\n" << io_grids[i][j] << "\n";
			}
		};

		// results
		const auto fill = [&](const result_t* v_root, const patch_t& p_end, bool out_only)
		{
			if(out_only)
			{
				v_root->local_patch.apply_fwd(orig_grid);
				_fill(io_idx::out);
				v_root->local_patch.apply_bwd(orig_grid);
			}
			else
			{

				v_root->local_patch.apply_fwd(orig_grid);
				_fill(io_idx::in_out);
				_fill(io_idx::in);
				v_root->local_patch.apply_bwd(orig_grid);

				p_end.apply_fwd(orig_grid);
				_fill(io_idx::in_out);
				_fill(io_idx::out);
				p_end.apply_bwd(orig_grid);
			}
		};

		const auto fill_row = [&](bool out_only)
		{
			std::set<std::pair<patch_t, patch_t>> result_pairs;
			// dfs
			for(const result_t* corr_root : res_graph.roots)
			{
				stack.emplace(corr_root, true);
				patch_t cur_patch;

				do if(!stack.top().second) {
					cur_patch -= stack.top().first->local_patch;
					stack.pop();
				}
				else {
					vertex_t& v_cur = stack.top();
					cur_patch += v_cur.first->local_patch;

					if(v_cur.first->children.empty()) {
						bool was_new = result_pairs.insert(std::make_pair(corr_root->local_patch, cur_patch)).second;
						if(was_new)
						{
							fill(corr_root, cur_patch, out_only);
							scc_sizes.push_back(
								((v_cur.first->exactness == exactness_t::equal) ? "$=" : "$\\ge ") +
								std::to_string(v_cur.first->scc_size) + "$");
						}
					}
					else for(const result_t* v_next : v_cur.first->children)
					 stack.emplace(v_next, true);

					v_cur.second = false;
				} while(!stack.empty()) ;
			}

			result_pairs.clear();
		};


		grid_t draw_grid = scc_finder.visited_grid();
		for(const point& p : draw_grid.points())
		if(draw_grid[p])
		 draw_grid[p] = -draw_grid[p];
		else
		{
			draw_grid[p] = orig_grid[p];
		}

		const std::string& m_rgb_32 = rgb32;

		// 1st grid
		o	<< "name\n\n" << "res_grid_" << stl_style_name(name) << "\n\n"
			<< "description\n\n" << "Gitterbelegung zum Modul \"" << name << "\"\n\n"
			<< "rgb32\n\nv:=(v>=0)?(" << m_rgb_32 << "):(255<<8)" << "\n\n" // TODO: customize
			<< "grids\n\n0\n\n"
			<< draw_grid << "\n"
			<< "path\n\n0\n\n\n\n\n";

		// 2nd grid
		std::size_t out_row_size = io_grids[(std::size_t)io_idx::out].size();
		o	<< "name\n\n" << "res_ini_" << stl_style_name(name) << "\n\n"
			<< "description\n\n" << "Initiale Ausgabezonen zum Modul \"" << name << "\" \n\n"
			<< "border\n\n"
			<< "rowsize\n\n" << out_row_size << "\n\n"
			<< "rgb32\n\n" << m_rgb_32 << "\n\n"
			<< "grids\n\n"
		;

		cb_caption(io_idx::out);
		_fill(io_idx::out);

		for(std::size_t i = 0; i < grid_counter; ++i)
		{
			o << ((i%out_row_size)?"":"path\n\n") << i << "\n\n";
			if(i < out_row_size)
			 o << "cur_rgb32=(255<<8)\n";
			o << '\n';
		}

		o << "\n\n";
		// TODO: only 3 \n


		// 3rd grid

		grid_counter = 0;

		o	<< "name\n\n" << "res_table_" << stl_style_name(name) << "\n\n"
			<< "description\n\n" << "Funktionstabelle zum Modul \"" << name << "\" \n\n"
			<< "border\n\n"
			<< "rowsize\n\n" << (row_size + 2) << "\n\n" // +2 : mapsto + scc size
			<< "rgb32\n\n" << m_rgb_32 << "\n\n"
			<< "grids\n\n"
		;

		// input captions
		cb_caption(io_idx::in_out);
		cb_caption(io_idx::in);
		// output captions
		cb_caption(io_idx::in_out);
		cb_caption(io_idx::out);

		fill_row(false);

		o << "text\n\n" << grid_counter << "\n\n" << "$\\mapsto$\n\n"
			<< grid_counter+1 << "\n\nin\n\n"
			<< grid_counter+2 << "\n\nout\n\n"
			<< grid_counter+3 << "\n\ni/o\n\n"
			<< grid_counter+4 << "\n\nscc-size\n\n"
			<< grid_counter+5 << "\n\n$\\;$\n\n";

		std::size_t i = 5;
		for(const std::string& s : scc_sizes)
		 o << grid_counter + (++i) << "\n\n" << s << "\n\n";

		// dump paths
		std::size_t middle = areas[(std::size_t)io_idx::in_out].size() +
			areas[(std::size_t)io_idx::in].size();

		for(std::size_t i = 0; i < grid_counter; ++i)
		{
			o << ((i%row_size)?"":"path\n\n") << i << "\n\n";
			if(i < row_size)
			{
				o << "cur_rgb32=(255<<8)\n";
				if(i == row_size - 1)
				{
					o << "\n" << grid_counter+4 << "\n\n";

					// extra line with in/out captions
					const auto cb = [&](const io_idx& idx) {
						std::size_t _idx = (std::size_t)idx;
						for(std::size_t j = 0; j < areas[_idx].size(); ++j)
						 o << "\n" << grid_counter+1+_idx << "\n\n";
					};
					o << "\npath\n";
					cb(io_idx::in_out);
					cb(io_idx::in);
					o << "\n" << grid_counter+5 << "\n\n"; // middle
					cb(io_idx::in_out);
					cb(io_idx::out);

					o << "\n" << grid_counter+5 << "\n\n";
				}
			}
			else if(!((i+1)%row_size))
			{
				o << "\n" << grid_counter + 5 + i/row_size << "\n\n";
			}

			if((i % row_size) == middle-1)
			{
				/*if(i < row_size)
				 o << "\n\n\n";
				else*/
				 o << "\n" << grid_counter + ((int)(i < row_size) * 5) << "\n\n";
			}
			o << '\n';
		}

		o << "\n\n";

		// stats

		o	<< "name\n\n" << "res_stats_" << stl_style_name(name) << "\n\n"
			<< "description\n\n" << "Statistiken zum Modul \"" << name << "\"\n\n"
			<< "rgb32\n\nv:=16777215" << "\n\n" // TODO: customize
			<< "text\n\n0\n\nKonfigurationen insgesamt:\n\n1\n\n" << stats.total + stats.sorted_out_2 + stats.sorted_out_1
			<< "\n\n2\n\ndavon nicht bewegbar:\n\n3\n\n" << stats.total + stats.sorted_out_2
			<< "\n\n4\n\ndavon nicht isolierbar:\n\n5\n\n" << stats.total
			<< "\n\n6\n\nLaufzeit (s):\n\n7\n\n" << "<" << stats.runtime_sec+1
			<< "\n\npath\n\n0\n\n\n1\n\n\n"
				"path\n\n2\n\n\n3\n\n\n"
				"path\n\n4\n\n\n5\n\n\n"
				"path\n\n6\n\n\n7\n\n\n\n\n";
	}
};

class MyProgram : public Program, types
{
#ifdef EN_BOOST_GRAPH
	void dump_depgraph(data& d)
	{
		std::cerr << "Dumping `depgraph.dot'" << std::endl;
		{
			d.dep_graph.remove_unconnected();
			std::ofstream ofs("depgraph.dot");
			ofs << d.dep_graph;
		}
	}

	void dump_paths(const data& d)
	{
		struct paths_v {
			patch_t patch;
			std::string label;
			paths_v(const patch_t& patch, int id) :
				patch(patch),
				label("id: " + std::to_string(id) + ", " + to_string(patch)) {}
			paths_v() {}
		};

		struct paths_e {
			std::string label;
		};

		using paths_graph_t = graph_base_t<paths_v, paths_e>;
		paths_graph_t paths_graph("paths");

		std::map<std::size_t, paths_graph_t::vertex_t> vertex_of;
		for(const result_t* v : d.res_graph.vertices)
		{
			std::cerr << "Adding: " << v->id << std::endl;
			vertex_of[v->id] = paths_graph.add_vertex(v->local_patch, v->vid);
		}

		for(const result_t* v : d.res_graph.vertices)
		for(const result_t* c : v->children)
		{
			std::cerr << "Adding edge: " << v->id << " -> " << c->id << std::endl;
			paths_graph.add_edge(vertex_of[v->id], vertex_of[c->id]);
		}

	}
#endif

	void eval(mode m)
	{
		data d;
		sca::io::deserializer des(std::cin);
		des >> d;
		switch(m)
		{
#ifdef EN_BOOST_GRAPH
			case mode::depgraph_dump:
				dump_depgraph(d);
				std::cerr << "Use `dot -Tpdf depgraph.dot > depgraph.pdf'" << std::endl;
				system("dot -Tpdf depgraph.dot");
				break;
			case mode::depgraph:
				dump_depgraph(d);
				system("dot -Tpdf depgraph.dot > depgraph.pdf");
				system("xdg-open depgraph.pdf");
				break;
			case mode::paths_dump:
				dump_paths(d);
				std::cerr << "Use `dot -Tpdf paths.dot > paths.pdf'" << std::endl;
				system("dot -Tpdf paths.dot");
				break;
			case mode::paths:
				dump_paths(d);
				system("dot -Tpdf paths.dot > paths.pdf");
				system("xdg-open paths.pdf");
				break;
#endif
			case mode::tex:
				d.dump_as_tex();
				break;
			case mode::diffs:
				d.results.print_results_pretty(d.orig_grid);
				break;
			case mode::used_points:
				std::cout << "All points that were ever activated:" << std::endl;
				std::cout << d.activated << std::endl;
				break;
			default: throw "invalid mode, use `help' as argv[1]";
		}
	}

	exit_t main()
	{
		mode m = mode::help;
		switch(argc)
		{
			case 2:
				m = modes[argv[1]];
				break;
			default:
				exit_usage();
		}

		if(m == mode::help)
		{
			std::cerr << "Possible modes:" << std::endl;
			modes.dump_names(std::cerr);
		}
		else
		{
			try {
				eval(m);
			}
			catch(const char* s)
			{
				std::cerr << "Aborting, reason: " << s << std::endl;
				return exit_t::failure;
			}
		}

		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "usr/eval <mode>"
		"";
	help.description = "Computes all end configurations "
		"using split algorithm.";
	help.input = "Dump file of split algorithm";
	help.output = "Information, human readable";
	help.add_param("mode", "type of information, use 'help' to get a list");

	MyProgram p;
	return p.run(argc, argv, &help);
}


