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

#include <algorithm>
#include "general.h"
#include "io/gridfile.h"
#include "utils/name_type_map.h"

const char* tex_header =
	"% created by sca toolsuite's io/tik\n"
	"% https://github.com/JohannesLorenz/sca-toolsuite\n\n"
	"\\documentclass{article}\n";
const char* tex_includes =
	"\\usepackage{tikz}\n"
	"\\usetikzlibrary{matrix}\n"
	"\\usepackage{xr}\n"; // cross refs for external docs
const char* tex_footer = "\\end{document}\n";

enum class tex_type
{
	include,
	complete,
	header,
	invalid
};

constexpr const sca::util::name_type_map_t<
	3, tex_type, tex_type::invalid> tex_type_map = {{
	{ "include", tex_type::include },
	{ "complete", tex_type::complete },
	{ "header", tex_type::header },
	// TODO: help ?
}};

class MyProgram : public Program
{

	static void dump_grid_as_tikz(const grid_t& g,
		const sca::io::color_formula_t& color_f,
		std::ostream& out = std::cout)
	{
		out << "\\draw[step=0.5cm,color=gray] (0, 0) grid ("
			<< ((float)g.dx()/2.0f) << ", " << ((float)g.dy()/2.0f) << ");\n";
			// note: >> 1 because of .5 cm
		out << "\\matrix[matrix of nodes,\n"
			<< "inner sep=0pt, anchor=south west,\n"
			<< "\tnodes={inner sep=0pt, text width=.5cm,\n"
			<< "\t\talign=center, minimum height=.5cm}]{\n";
		const auto draw_node = [&](std::size_t x, std::size_t y){
			out << "\\node[s_" << color_f(g, point(x, y)) << "]{" << g[point(x, y)] << "};";
		};
		for(std::size_t y = 0; y < g.dy(); ++y)
		{
			out << "\t";
			for(std::size_t x = 0; x < g.dx() - 1; ++x)
			 draw_node(x, y), out << " & ";
			draw_node(g.dx()-1, y), out << " \\\\" << std::endl;
		}
		out << "};\n";
	}

	// returns point in tenth of mm
	static point to_tikz(const point& p, const dimension& dim) {
		const auto shift = [](const int x) { return ((x << 1) + 1) * 25; };
		return point(shift(p.x), dim.dy() * 50 - shift(p.y));
	}

	// returns point in mm
	static void dump_mm_point(const point& p, std::ostream& out) {
		out << "(" << (float) p.x / 100.0f << ", "
			<< (float) p.y / 100.0f << ")";
	}

	static void make_tikz(const tex_type& tt, const sca::io::gridfile_t& gridfile,
		std::ostream& out = std::cout)
	{
		if(tt == tex_type::complete)
		{
			out << tex_header;
			out << tex_includes;
			out << "\\begin{document}\n\n";
		}

	//	out << gridfile.value<std::string>("description");

		std::size_t max_width = 0;

		const auto& grids = gridfile["grids"];

		for(std::size_t i = 0; i < grids.max(); ++i)
		 max_width = std::max(max_width, (std::size_t)grids.value<grid_t>(i).dx());

		constexpr const std::size_t page_mm = 197, cell_mm = 5;

		const std::size_t max_possible_rowsize = page_mm / ((max_width + 3) * cell_mm);

		std::size_t max_pathlen = 0;
		for(std::size_t i = 0; i < gridfile.max(); ++i)
		{
			max_pathlen = std::max(max_pathlen, gridfile[i].max());
		}

		const std::size_t rowsize = std::min(max_possible_rowsize, max_pathlen);

		sca::io::color_formula_t color(
			gridfile.value<std::string>("rgb32").c_str());
		std::set<int32_t> color_table; // collection of all colors
		for(std::size_t i = 0; i < grids.max(); ++i)
		{
			const grid_t& g = grids.value<grid_t>(i);
			for(const point& p : g.points())
			 color_table.insert(color(g, p));
		}

		for(const int32_t& i : color_table)
		out << "\t\\definecolor{rgb" << i << "}{RGB}{"
			<< ((i>>16) & 255) << ", "
			<< ((i>>8) & 255) << ", "
			<< (i & (255)) << "}\n";

		for(std::size_t i = 0; i < gridfile.max(); ++i)
		{
			const sca::io::supersection_t& path = gridfile[i];
		/*
			const std::size_t factor = path.max() / rowsize;
			const std::size_t colsize = factor + ((path.max() - rowsize * factor) > 0);*/

			/*for(int col = 0; col < rowsize; ++col)
			{
				const grid_t& g = grids.value<grid_t>(path.value<);

				dump_grid_as_tikz(g, out);

			}*/

			out << "\\begin{figure}\n";
			out << "\\centering\n";
			out << "\\begin{tabular}{";
			for(std::size_t i = 0; i < rowsize; ++i)
			 out << 'c';
			out << "}\n";

			std::size_t col = 0;


			for(const auto& pr : path.numbered_values<sca::io::path_node>())
			{
				//std::cerr << node.key() << " --> " << node.value().description << std::endl;

			//	out << "\\begin{figure}\n\\begin{tikzpicture}\n";
				out << "\\begin{tikzpicture}\n[\n";
				for(const int32_t& i : color_table) {
					out << "\ts_" << i << "/.style={top color=rgb" << i << ", bottom color = rgb" << i << "},\n";
				}
				out << "\tdummy/.style={}";
				out << "]\n";

				const sca::io::path_node& node = pr.value();
				const grid_t& grid = grids.value<grid_t>(pr.key());

				dump_grid_as_tikz(grid, color, out);
				for(const sca::io::path_node::arrow ar : node.arrow_list)
				{
					point tp1 = to_tikz(ar.p1, grid.human_dim()),
						tp2 = to_tikz(ar.p2, grid.human_dim());
					out << "\\draw [->, thick] ";
					dump_mm_point(tp1, out);
					out << " -- ";
					dump_mm_point(tp2, out);
					out << ";\n";
				}


			//	out << "\\end{tikzpicture}\n\\caption{TODO TODO TODO}\n\\end{figure}\n";
				out << "\\end{tikzpicture}";

				bool linebreak = (!(++col%rowsize));
				out << (linebreak ? " \\\n" : " &\n"); // TODO: in one line?
			}

			out << "\\end{tabular}\n";
			out << "\\caption{" << gridfile.value<std::string>("description") << "}\n";
			out << "\\label{" << gridfile.value<std::string>("name") << '_' << i << "}\n";
			out << "\\end{figure}\n";
		}


		if(tt == tex_type::complete)
		 out << tex_footer;
	}

	exit_t main()
	{
		//const char* fname = nullptr;
		tex_type tt = tex_type::invalid;
		switch(argc)
		{
			case 2:
				tt = tex_type_map[argv[1]];
			case 1:
				break;
			default:
				return exit_usage();
		}

		if(tt == tex_type::header)
		{
			std::cout << tex_includes;
		}
		else
		{
			sca::io::secfile_t inf;
			sca::io::gridfile_t gridfile;
			try {
				gridfile.parse(inf);
			} catch(sca::io::secfile_t::error_t ife) {
				std::cout << "infile line " << ife.line
					<< ": "	 << ife.msg << std::endl;
			}
			make_tikz(tt, gridfile);
		}

		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "io/tik [<?>]";
	help.description = "Converts sca formatted grid file to tikz";
	help.input = "sca formatted grid file";
	help.output = "minimal latex file";
//	help.add_param("infile", "specifies a file to read a grid from");

	MyProgram p;
	return p.run(argc, argv, &help);
}
