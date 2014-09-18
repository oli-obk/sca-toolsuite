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

#include "io/secfile.h"
#include "general.h"
#include "ca_basics.h"
#include "ca_convert.h"

using namespace sca::ca;

class color_t
{
public:
	char r = 0, g = 0, b = 0, a = 255;
	color_t(const char* str)
	{ // TODO: const member construction...
		switch(*str)
		{ // TODO: not use assert!
			case 'g': assert(!strncmp(str, "green", 5)); g = 255; break;
			case 'r': assert(!strncmp(str, "red", 3)); r = 255; break;
			case 'b': assert(!strncmp(str, "blue", 4)); b = 255; break;
			default: throw "Unknown color type";
		}
	}
};

class path_node
{
	inline static const char* next_word(const char*& str) {
		while(*(++str) != ' ') ;
		return ++str;
	}
public:
	//int grid_id;
	std::string description;

	enum class mark_type
	{
		active,
		formula
	};

	class markup
	{
	public:
		color_t color;
		mark_type tp;
		std::string formula;
		markup(const char* str) :
			color(str)
		{ // TODO: const constr
			const char* ptr = str;
			next_word(ptr);
			tp = (!strcmp(ptr, "active"))
				? mark_type::active
				: mark_type::formula;
			if(tp == mark_type::formula)
			 formula = ptr;
		}
	};

	class arrow
	{
		point p1, p2;
	public: // TODO: const ctor
		arrow(const char* str)
		{
			const char* ptr = str;
			p1.x = atoi(ptr); next_word(ptr);
			p1.y = atoi(ptr); next_word(ptr);
			p2.x = atoi(ptr); next_word(ptr);
			p2.y = atoi(ptr);
		}
	};

	std::vector<markup> markup_list;
	std::vector<arrow> arrow_list;

	bool parse(io::secfile_t& inf)
	{
		/*if(inf.read_int(grid_id))
		{
			inf.read_newline();*/
			std::string str;
			while((str = inf.read_string()).size())
			{
				if(!str.compare(0, 4, "mark"))
				 markup_list.emplace_back(str.data() + 5);
				else if(!str.compare(0, 5, "arrow"))
				 arrow_list.emplace_back(str.data() + 6);
				else
				{
					assert(description.empty());
					description = str;
				}
			}
			return true;
		//}
		//else return false;
	}

	void dump(std::ostream& stream) const { (void)stream; /*TODO*/ }
};

namespace io
{

template<> // TODO: abstract case of path_node? enable_if?
class leaf_template_t<path_node> : public leaf_base_t
{
	path_node t;
public:
	void parse(io::secfile_t& inf) { t.parse(inf); }
	void dump(std::ostream& stream) const { t.dump(stream); }
};

}

class scene_grids_t : public io::supersection_t
{
public:
	scene_grids_t() : supersection_t(type_t::multi) {
		init_factory<io::leaf_template_t<grid_t>>();
	}
};

class scene_path_t : public io::supersection_t
{
public:
	scene_path_t() : supersection_t(type_t::multi) {
		init_factory<io::leaf_template_t<path_node>>();
	}
};


class scene_t : public io::supersection_t // TODO: public?
{
public:
	scene_t() : supersection_t(type_t::batch)
	{
		init_leaf<io::leaf_template_t<std::string>>("description");
		init_leaf<io::leaf_template_t<n_t>>("n");
		init_subsection<scene_grids_t>("grids");

		init_factory<scene_path_t>();
		set_batch_str("path");
	}

/*	void get_tv()
	{


		tv_ctor cons(n);
		for(const std::vector<path_node>& v : paths)
		{
			using itr_t = std::vector<path_node>::const_iterator;
			itr_t tar = v.begin();
			itr_t src = tar++;
			for(; tar != v.end(); ++src, ++tar)
			{
				cons.add_eq(grids[src->grid_id], grids[tar->grid_id]);
			}
		}

		tv = std::move(trans_vector_t(std::move(cons)));
	} */
};


#if 0
class scene_t
{
public:
	std::string description;
	n_t n;
	std::map<std::size_t, grid_t> grids;
	std::vector<std::vector<path_node>> paths;
	trans_vector_t tv;



	void dump(std::ostream& outf)
	{
		outf << "description" << std::endl
			<< description << std::endl
			<< "n" << std::endl
			<< n << std::endl
			<< "grids" << std::endl;


	//	outf << section("description", description)
	//		<< section("grids", );
	}

	void parse(secfile_t& inf)
	{
		section_t sec;
		while((sec = inf.read_section()))
		{
			int idx;
			if(sec == "description")
			 description = inf.read_section();
			else if(sec == "n")
			 inf.stream >> n;
			else if(sec == "grids")
			{
				while (inf.read_int(idx)) // TODO: these ints could be sections, too
				{
					grids[idx] = grid_t(inf.stream, 0);
				}
			}
			else if(sec == "path")
			{
				std::vector<path_node> path;
				path_node node;
				while (node.parse(inf))
				{
					path.emplace_back(node);
					node = path_node();
				}
				paths.emplace_back(path);
			}
		}
	}

	void get_tv()
	{
	/*	tv_ctor cons(n);
		for(const std::vector<path_node>& v : paths)
		{
			using itr_t = std::vector<path_node>::const_iterator;
			itr_t tar = v.begin();
			itr_t src = tar++;
			for(; tar != v.end(); ++src, ++tar)
			{
				cons.add_eq(grids[src->grid_id], grids[tar->grid_id]);
			}
		}

		tv = std::move(trans_vector_t(std::move(cons)));*/
	}
};
#endif

class MyProgram : public Program
{
	exit_t main()
	{
		/*
		 * args
		 */

		switch(argc)
		{
			case 2:
			//	tbl_file = argv[1];
			// TODO: output without changes?
				assert_usage(!strcmp(argv[1], "tex"));
				break;
			case 1: break;
			default:
				exit_usage();
		}

		/*
		 * parsing
		 */

		io::secfile_t inf;
		scene_t scene;
		try {
			scene.parse(inf);
		} catch(io::secfile_t::error_t ife) {
			std::cout << "infile line " << ife.line << ": "	 << ife.msg << std::endl;
		}
		std::cout << scene;

		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "ca/scene [<out format>]"
		"";
	help.description = "Converts a scene into a document file."
		"";
	help.input = "Input grid in a special format.";
	help.output = "The ca document";
	help.add_param("<out format>", "output format. currently only: tex (=default)");

	MyProgram p;
	return p.run(argc, argv, &help);
}

