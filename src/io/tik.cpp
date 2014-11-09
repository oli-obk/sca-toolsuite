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

#include <iomanip>
#include <algorithm>
#include "general.h"
#include "io/gridfile.h"
#include "utils/name_type_map.h"
#include "io/latex.h"

const char* tex_header =
	"% created by sca toolsuite's io/tik\n"
	"% https://github.com/JohannesLorenz/sca-toolsuite\n\n"
	"\\documentclass{scrreprt} % has wide borders\n";
const char* tex_includes =
#ifdef USE_TIKZ
	"\\usepackage{tikz}\n"
	"\\usetikzlibrary{matrix}\n"
#else
	"\\usepackage[table]{xcolor} % http://ctan.org/pkg/xcolor\n"
#endif
	"\\usepackage{graphicx} % \\scalebox\n"
	"\\usepackage{subcaption}\n"
	"\\usepackage[ngerman]{babel}\n"
	//"\\newcolumntype{C}[1]{>{\\raggedright\\let\\newline\\\\\\arraybackslash\\hspace{0pt}}m{#1}}"
	"\\usepackage{array}\n"
	"\\newcommand{\\cb}[1]{\\cellcolor[HTML]{#1}\\textcolor{black}}\n"
	"\\newcommand{\\cw}[1]{\\cellcolor[HTML]{#1}\\textcolor{white}}\n"

	;
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
#if 0
		// draw grid
		out << "\\draw[step=0." << cell_mm << "cm,color=gray] (0, 0) grid ("
			<< ((float)g.dx()*(float)cell_mm/10.0f) << ", "
			<< ((float)g.dy()*(float)cell_mm/10.0f) << ");\n";

		// draw nodes, incl values and colors
		out << "\\matrix[matrix of nodes,\n"
			<< "inner sep=0pt, anchor=south west,\n"
			<< "\tnodes={inner sep=0pt, text width=." << cell_mm << "cm,\n"
			<< "\t\talign=center, minimum height=."  << cell_mm << "cm}";
	/*	for(const grid_t::line& l : g.lines())
		{
			for(const point& p : l)
			{

			}
		}*/

		out <<"]{\n";
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
#else
		const auto draw_node = [&](std::size_t x, std::size_t y) {
			int32_t color_val = ((color_f(g, point(x, y))) & (0xFFFFFF));
			auto number = g[point(x, y)];
			if(color_val)
			out << (sca::io::color_formula_t::text_black(color_val) ? "\\cb{" : "\\cw{")
				<< std::hex << std::setfill('0') << std::setw(6) << std::uppercase
				<< color_val
				<< std::dec
				<< "} ";
			if(number < 0 || number > 9)
			 out << '{';
			if(number >= 0)
			 out << number;
			else
			{
				out << "\\scalebox{0.25}[1.0]{\\(-\\)}";
				out << -number;
			}
			if(number < 0 || number > 9)
			 out << '}';
		};
		out << "\\centering\n";
		out << "\\begingroup\n";
		out << "\\centering\n";
		out << "\\renewcommand{\\arraystretch}{1.0}\n";
		out << "\\centering\n";
		out << "\\small\\tabcolsep=0.00cm\\begin{tabular}{";
		for(std::size_t x = 0; x < g.dx(); ++x)
		 //out << "C{3mm}";
		 out << ">{\\centering\\arraybackslash}p{3mm} ";
		out << "}\n";
		for(std::size_t y = 0; y < g.dy(); ++y)
		{
			out << "\t";
			for(std::size_t x = 0; x < g.dx() - 1; ++x)
			 draw_node(x, y), out << " & ";
			draw_node(g.dx()-1, y), out << " \\\\" << std::endl;
		}
	//	out << "\\caption{" << "xy" << "}\n";
		out << "\\end{tabular}\n";
		out << "\\endgroup\n";
#endif


	}

	// returns point in tenth of mm
	static point to_tikz(const point& p, const dimension& dim) {
		const auto shift = [](const int x) { return ((x << 1) + 1) * 25; };
		return point(shift(p.x), dim.dy() * cell_mm * 10 - shift(p.y));
	}

	// returns point in mm
	static void dump_mm_point(const point& p, std::ostream& out) {
		out << "(" << (float) p.x / 100.0f << ", "
			<< (float) p.y / 100.0f << ")";
	}

	class tex_str
	{
		const std::string& str;
	public:
		tex_str(const std::string& str) : str(str) {}
		friend std::ostream& operator<<(std::ostream& stream, const tex_str& src)
		{
			std::size_t num = 0;
			for(const char& c : src.str)
			{
				if(c == '"')
				 stream << (((++num)%2) ? "\\glqq{}" : "\\grqq{}");
				else
				 stream << c;
			}
			return stream;
		}
	};

	static void make_tikz_content(const sca::io::gridfile_t& gridfile,
		std::ostream& out = std::cout)
	{
	//	out << gridfile.value<std::string>("description");

		const auto& grids = gridfile["grids"];

		std::size_t rowsize;
		std::size_t mod_val = gridfile.value<int>("rowsize_mod", 1);

		const auto lower_to_rowsize_mod = [&mod_val](std::size_t& val) {
			val -= val % mod_val;
		};

		const auto upper_to_rowsize_mod = [&](std::size_t& val) {
			lower_to_rowsize_mod(val);
			val += ((val % mod_val) != 0) * mod_val;
		};

		const auto rowsize_hint = gridfile.leaf<int>("rowsize");
		if(rowsize_hint.is_read())
		{
			rowsize = rowsize_hint.value();
			lower_to_rowsize_mod(rowsize);
		}

		const bool paths_close = gridfile.leaf<void>("paths_close").is_read();

		std::size_t max_possible_rowsize, max_width = 0;
		{
			for(std::size_t i = 0; i < grids.max(); ++i)
			 max_width = std::max(max_width, (std::size_t)grids.value<grid_t>(i).dx());

			max_possible_rowsize = page_mm / ((max_width + 1) * cell_mm);
			lower_to_rowsize_mod(max_possible_rowsize);

			std::size_t max_pathlen = 0;
			for(std::size_t i = 0; i < gridfile.max(); ++i)
			{
				max_pathlen = std::max(max_pathlen, gridfile.value<std::vector<sca::io::path_node>>(i).size());
			}

			if(!rowsize_hint.is_read())
			{
				rowsize = max_possible_rowsize;
				if(paths_close)
				{
					std::size_t cur_max_x = 0;
					std::size_t cur_x = 0;
					for(std::size_t i = 0; i < gridfile.max(); ++i)
					{
						std::size_t path_len = gridfile.value<std::vector<sca::io::path_node>>(i).size();
						std::size_t theor_pos = (cur_x + path_len);
						if(theor_pos >= cur_max_x)
						{
							cur_max_x = std::max(cur_max_x, cur_x);
							cur_x = path_len;
						}
						else
						 cur_x = theor_pos;
					}
					upper_to_rowsize_mod(cur_max_x);
					max_possible_rowsize = cur_max_x;
				}
				else
				{
					std::size_t tmp = max_pathlen;
					upper_to_rowsize_mod(tmp);
					rowsize = std::min(rowsize, max_pathlen);
				}
				if(!rowsize)
				{
					std::cerr << "WARNING: grid is probably too width." << std::endl;
					std::cerr << "max_possible_rowsize: " << max_possible_rowsize << std::endl;
					std::cerr << "max_pathlen: " << max_pathlen << std::endl;
					rowsize = mod_val;
				}
			}
		}

		const bool borders = gridfile.leaf<void>("border").is_read();

		sca::io::color_formula_t main_color(
			gridfile.value<std::string>("rgb32").c_str());

		const auto new_tbl_hdr = [&]()
		{
			out << "\\begin{table}[H]\n";
			out << "\\renewcommand{\\arraystretch}{2.0}\n";
			out << "\\centering\n";

			out << "\\begin{tabular}{";
			if(borders) {
				out << '|';
				for(std::size_t i = 0; i < rowsize; ++i) out << "c|";
			}
			else for(std::size_t i = 0; i < rowsize; ++i)
			 out << 'c';
			out << "}\n";

			if(borders)
			 out << "\\hline\n";
		};

		const auto new_tbl_ftr = [&](std::size_t page_no)
		{
			if(borders)
			 out << " \\\\\n\\hline\n";

			out << "\\end{tabular}\n";
			if(page_no) {
				out << "\\caption{Fortsetzung von Tabelle \\ref{" << tex_str(gridfile.value<std::string>("name"));
				if(page_no > 1)
				 out << "_" << page_no - 1;
				out << "}.}";
			}
			else
			 out << "\\caption{" << tex_str(gridfile.value<std::string>("description")) << "}\n";

			out << "\\label{" << tex_str(gridfile.value<std::string>("name"));
			if(page_no)
				out << "_" << page_no;
			out << "}\n";

			out << "\\end{table}\n";
		};

		new_tbl_hdr();

		std::size_t max_rows = 1 << 3; // TODO
		std::size_t col = 0, row = 0, page = 0;

		for(std::size_t i = 0; i < gridfile.max(); ++i)
		{
			const std::vector<sca::io::path_node>& path = gridfile.value<std::vector<sca::io::path_node>>(i);
		/*
			const std::size_t factor = path.max() / rowsize;
			const std::size_t colsize = factor + ((path.max() - rowsize * factor) > 0);*/

			/*for(int col = 0; col < rowsize; ++col)
			{
				const grid_t& g = grids.value<grid_t>(path.value<);

				dump_grid_as_tikz(g, out);

			}*/

			int node_id = 0;

			if(!paths_close || (col + path.size() >= rowsize))
			 col = 0;

			for(const sca::io::path_node& node : path)
			{
				if(i || col) // no linebreak before start
				{
					std::cerr << "col: " << col << std::endl;
					bool linebreak = (col == 0);
					out << (linebreak ? " \\\\\n" : " &\n");
					if(borders && linebreak)
					 out << "\\hline\n";

					row += linebreak;
					bool pagebreak = (row >= max_rows);
					if(pagebreak)
					{
						new_tbl_ftr(page);
						row = 0;
						++page;
						new_tbl_hdr();
					}
				}


				//std::cerr << node.key() << " --> " << node.value().description << std::endl;

			//	out << "\\begin{figure}\n\\begin{tikzpicture}\n";


				const bool has_grid = grids.numbered_values<grid_t>().find(node.grid_id) != grids.numbered_values<grid_t>().end();
				const grid_t& grid = has_grid ? grids.value<grid_t>(node.grid_id) : grid_t();
				const bool special_color = !node.cur_color.empty();

				sca::io::color_formula_t cur_color(
					special_color ? node.cur_color.c_str() : "v"
				);
				sca::io::color_formula_t& the_color = special_color ? cur_color : main_color;


				std::set<int32_t> color_table; // collection of all colors
				//for(std::size_t i = 0; i < grids.max(); ++i)
				if(has_grid)
				{
					const grid_t& g = grid;
					for(const point& p : g.points())
					 color_table.insert(the_color(g, p));
				}

#if 0
				for(const int32_t& i : color_table)
				out << "\\definecolor{rgb" << i << "}{RGB}{"
					<< ((i>>16) & 255) << ", "
					<< ((i>>8) & 255) << ", "
					<< (i & (255)) << "}\n";


				out << "\\begin{tikzpicture}\n[\n";
				for(const int32_t& i : color_table) {
					out << "\ts_" << i << "/.style={top color=rgb" << i << ", bottom color = rgb" << i << "},\n";
				}
				out << "\tdummy/.style={}";
				out << "]\n";

				dump_grid_as_tikz(grid, the_color, out);
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
#else

			//	out << "\\begin{figure}\n";
			//	out << "\\begin{table}\n";

				if(has_grid) // (TODO: hack)
				{
					out << "\\begin{subtable}[c]{" << (grid.dx() * cell_mm + 2) << "mm}\n";
					dump_grid_as_tikz(grid, the_color, out);
					if(node.description.size())
					 out << "\\small\\caption*{" << node.description << "} \\ \n"; // force newline for more distance
					out << "\\label{" << tex_str(gridfile.value<std::string>("name"))
						<< "_grid_" << i << '_' << node_id << "}\n";
					out << "\\end{subtable}";
				}
				else
				{
					out << gridfile["text"].value<std::string>(node.grid_id) << "\n";
				}


			//	out << "\\captionof{table}{" << "xy" << "}\\label{xy}\n";

			//	out << "\\end{table}\n";

			//	out << "\\label{" << "xy" << /*'_' << i <<*/ "}\n";
			//	out << "\\end{figure}\n";

			/*	if(node.description.size())
				{
					out << "\\\\\n" << node.description;
				}*/

				++node_id;
#endif
				++col;
			}
		}

		new_tbl_ftr(page);


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

		exit_t exit_code = exit_t::success;

		if(tt == tex_type::invalid)
		{
			throw "specify arg 1: include, complete, header";
		}
		if(tt == tex_type::header)
		{
			std::cout << tex_includes;
		}
		else
		{
			if(tt == tex_type::complete)
			{
				std::cout << tex_header;
				std::cout << tex_includes;
				std::cout << "\\begin{document}\n\n";
			}

			sca::io::secfile_t inf;
			try {
				sca::io::gridfile_t gridfile;
				while(inf >> gridfile) {
					std::cerr << "... read one gridfile."<< std::endl;
					make_tikz_content(gridfile);
					gridfile.clear(); // TODO
				}
			} catch(sca::io::secfile_t::error_t ife) {
				std::cerr << "infile line " << ife.line
					<< ": "	 << ife.msg << std::endl;
				exit_code = exit_t::failure;
			}

			if(tt == tex_type::complete)
			 std::cout << tex_footer;
		}

		return exit_code;
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

