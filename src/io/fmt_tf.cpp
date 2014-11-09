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
#include "io/gridfile.h"
#include "ca_convert.h" // TODO!
#include "io/latex.h"

// TODO: common hdr
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


class MyProgram : public Program
{
	void make_new_gridfile(const sca::grid_io::gridfile_t& data)
	{
		std::ostream& o = std::cout;
		const std::string& name = data.value<std::string>("name");

		bool m_used = false, r_used = false;
		for(std::size_t i = 0; i < data.max(); ++i)
		{
			const sca::io::supersection_t& group = data[i];
			m_used = m_used || group.leaf<void>("mirror").is_read();
			r_used= r_used || group.leaf<void>("rotate").is_read();
		}

		o	<< "name\n\n" << "tf_grid_" << stl_style_name(name) << "\n\n"
			<< "description\n\n" << "Überführungsfunktion für den Zellularautomaten \"" << name << "\".";
		if(r_used || m_used)
		{
			o << ' ';
			if(r_used)
			 o << "$\\circlearrowright$ bedeutet Drehsymmetrie mit $\\{0..3\\} \\cdot 90^\\circ$";
			if(r_used && m_used)
			 o << ", ";
			if(m_used)
			 o << "$|$ bedeutet Spiegelsymmetrie";
			o << '.';
		}
		o	<<"\n\n"
			<< "rowsize_mod\n\n3\n\n"
			<< "paths_close\n\n"
			<< "rgb32\n\n" << data.value<std::string>("rgb32", "v:=255+(255<<8)+(255<<16)") << "\n\n" // TODO: customize
			<< "grids\n\n"
		;

		std::size_t grid_id = 0;
		// evaluate graph, fill in/out cells
		for(std::size_t i = 0; i < data.max(); ++i)
		{
			const sca::io::supersection_t& group = data[i]; // TODO: named_values does not work?
			for(const auto& gp2 : group.numbered_values<sca::grid_io::grid_pair_t>())
			{
				const sca::grid_io::grid_pair_t& gpr = gp2.value();
				o << grid_id << "\n\n" << gpr.in << std::endl
					<< grid_id+1 << "\n\n" << gpr.out << std::endl;
				grid_id += 2;
			}
		}

		std::size_t mapsto_id = grid_id;
		grid_id = 0;
		// evaluate graph, fill in/out cells
		for(std::size_t i = 0; i < data.max(); ++i)
		{
			const sca::io::supersection_t& group = data[i]; // TODO: named_values does not work?
			bool m = group.leaf<void>("mirror").is_read(),
				r = group.leaf<void>("rotate").is_read();
			std::string mr_str = (m ? "$|$ " : "");
			mr_str += (r ? "$\\circlearrowright$" : "");
			for(const auto& gp2 : group.numbered_values<sca::grid_io::grid_pair_t>())
			{
				(void)gp2;
				o << "path\n\n"
					<< grid_id << "\n\n";
				o << "\n" << mapsto_id << "\n\n\n"
					<< grid_id+1 << "\n\n";
				if(mr_str.size())
					o << mr_str << "\n\n";
				o << "\n";
				grid_id += 2;
			}
		}

		o << "text\n\n" << grid_id << "\n\n$\\mapsto$\n\n";

		o << "\n\n";
	}


	exit_t main()
	{
		assert_usage(argc == 1);

		sca::io::secfile_t inf;
		try {
			sca::grid_io::gridfile_t gridfile;
			while(inf >> gridfile) {
				std::cerr << "Read, success." << std::endl;
				make_new_gridfile(gridfile);
				gridfile.clear();
			}
		} catch(sca::io::secfile_t::error_t ife) {
			std::cerr << "infile line " << ife.line
				<< ": "	 << ife.msg << std::endl;
			return exit_t::failure;
		}

		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "usr/fmt_tf"
		"";
	help.description = "Computes all end configurations "
		"using split algorithm.";
	help.input = "Dump file of split algorithm";
	help.output = "Information, human readable";
	help.add_param("mode", "type of information, use 'help' to get a list");

	MyProgram p;
	return p.run(argc, argv, &help);
}


