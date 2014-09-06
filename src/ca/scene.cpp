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
#include "ca_basics.h"
#include "ca_convert.h"

#include <string>
#include <iostream>
#include <cstring>
#include <map>

class section_t
{
	std::string _value;
public:
	operator bool() { return _value.size(); }
	bool operator==(const char* str) const { return _value == str; }
	//section_t(const char* name) : name(name) {}
	section_t(std::string&& value) : _value(value) {}
	section_t() {}
//	explicit operator const std::string&() const { return value; }
	const std::string& value() const { return _value; }
	friend std::ostream& operator<< (std::ostream& stream,
		const section_t& s) {
		return stream << s._value;
	}
};

bool is_number_digit(const char& c) noexcept {
	return (isdigit(c) || c == '-' || c == '+');
}

void skip_num_after(const char* ptr) noexcept
{
	while(is_number_digit(*++ptr)) ;
}


bool is_number(const char* ptr) noexcept
{
	bool ok = is_number_digit(*ptr);
	while(is_number_digit(*++ptr)) ;
	return ok && !*(ptr);
}

struct infile_t
{
	constexpr static std::size_t READ_BUF_SIZE = 128;
	char read_buffer[READ_BUF_SIZE] = { 0 };
	std::istream& stream;
public:

	struct error_t
	{
		int line;
		const char* msg;
	};

	int line = 1;

	error_t mk_error(const char* err) const {
		std::cerr << "parsing stopped after: " << read_buffer << std::endl;
		return error_t { line, err };
	}

	infile_t(std::istream& stream = std::cin) :
		stream(stream) {}

	void read_newline()
	{
		if(*get_next_line()) {
			throw mk_error("Expected newline");
		}
		++line;
	}

	static bool m_atoi(int& res, const char* str) {
		return is_number_digit(*str)
			? res = atoi(str), true
			: false;
	}

	bool read_int(int& i)
	{
		const char* buf = get_next_line();
		/*if(!*buf) {
			throw mk_error("Expected a non-empty number");
		}*/
		++line;
		return (m_atoi(i, buf))
			? clear_buffer(), true
			: false;
	}

	const char* get_next_line()
	{
		const char* result = read_buffer;
		if(!*result) {
			stream.getline(read_buffer, READ_BUF_SIZE);
			++line;
			if(!stream.good())
			 clear_buffer();
		}
		return result;
	}

	void clear_buffer() { *read_buffer = 0; }

	std::string read_string_noclear()
	{
		/*if(!*read_buffer) {
			throw mk_error("Expected a non-empty string");
		}*/
		return ++line, std::string(get_next_line());
	}

	std::string read_string()
	{
		/*if(!*read_buffer) {
			throw mk_error("Expected a non-empty string");
		}*/
		std::string result = read_string_noclear();
		clear_buffer();
		return result;
	}

	bool match_string(const char* str)
	{
		/*if(!*read_buffer) {
			throw mk_error("Expected a non-empty string");
		}*/
		std::string result = read_string_noclear();
		if(result == str)
		{
			clear_buffer();
			return true;
		}
		else
		 return false;
	}

	/*void read_grid(grid_t& grid) {
		stream >> grid;
	}*/

	section_t read_section()
	{
		std::string res = read_string_noclear();
		//if(res.empty())
		//throw mk_error("Expected section, got empty line!");
		if(!stream.good())
		 res = "";
		if(res.size()) // i.e. this was a section
		{
			clear_buffer();
			read_newline();
		}
		return std::move(res);
	}
};

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

inline const char* next_word(const char*& str) {
	while(*(++str) != ' ') ;
	return ++str;
}

class path_node
{
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

	bool parse(infile_t& inf)
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
};

class leaf_base_t
{
public:
	virtual void parse(infile_t& inf) = 0;
};

template<class T>
class leaf_template_t : public leaf_base_t
{
	T t;
public:
	void parse(infile_t& inf) { inf.stream >> t; std::cerr << "Read object via cin: " << t << std::endl; }
};

template<>
class leaf_template_t<std::string> : public leaf_base_t
{
	std::string t;
public:
	void parse(infile_t& inf) { t = inf.read_section();  std::cerr << "Read string: " << t << std::endl; }
};

template<> // TODO: abstract case of path_node? enable_if?
class leaf_template_t<path_node> : public leaf_base_t
{
	path_node t;
public:
	void parse(infile_t& inf) { t.parse(inf); }
};

class factory_base
{
public:
	virtual leaf_base_t* make() = 0;
};

template<class T>
class factory : public factory_base
{
public:
	virtual T* make() { return new T(); }
};

class supersection_t : public leaf_base_t
{
public:
	using self_type = supersection_t;

	enum class type_t
	{
		sections,
		multi,
		batch
	};
	type_t type;
private:
	const bool required;
	std::string batch_str;
	factory_base* leaf_factory;
	std::map<std::string, leaf_base_t*> supersections;
	std::map<std::size_t, leaf_base_t*> multi_sections;
	std::map<std::string, leaf_base_t*> leafs;
protected:
	template<class T>
	void init_subsection(const char* sec_name) {
		supersections[sec_name] = new T();
	}
	template<class T>
	void init_leaf(const char* leaf_name) {
		leafs[leaf_name] = new T();
	}
	template<class T>
	void init_factory() {
		leaf_factory = new factory<T>();
	}
	void set_batch_str(const char* str) {
		batch_str = str;
	}

	enum class cur_type_t
	{
		multi,
		batch,
		super,
		leaf,
		unknown
	};

	mutable std::map<std::string, leaf_base_t*>::iterator super_itr;
	mutable std::map<std::string, leaf_base_t*>::iterator leaf_itr;

	cur_type_t check_string(infile_t& inf, std::string& s)
	{
		s = inf.read_string_noclear();
		cur_type_t res;
		if(type == type_t::multi && is_number(s.c_str()))
		{
			std::cout << "Found multi object: `" << s << "'" << std::endl;
			res = cur_type_t::multi;
		}
		else if((type == type_t::batch) && (batch_str == s))
		{
			std::cout << "Found batch string: `" << s << "'" << std::endl;
			res = cur_type_t::batch;
		}
		else if((super_itr = supersections.find(s)) != supersections.end())
		{
			std::cout << "Found supersection: `" << s << "'" << std::endl;
			res = cur_type_t::super;
		}
		else if((leaf_itr = leafs.find(s)) != leafs.end()) // TODO: store itr in mutable class var?
		{
			std::cout << "Found leaf: `" << s << "'" << std::endl;
			res = cur_type_t::leaf;
		}
		else {
			std::cout << "No match: `" << s << "'" << std::endl;
			res = cur_type_t::unknown;
		}

		if(res != cur_type_t::unknown)
		{
			inf.clear_buffer();
			inf.read_newline();
		}

		return res;
	}


public:
	void parse (infile_t& inf)
	{
		std::string s;
		cur_type_t cur;
		while(cur_type_t::unknown != (cur = check_string(inf, s))) // TODO: while type = inf.read_string_no_clear() ...
		{
			std::cerr << "Trying to parse section: " << s << std::endl;
			int idx = -1; // TODO: size_t


			switch(cur)
			{
				case cur_type_t::multi:
				{
					idx = std::stoi(s);
					std::cerr << "Reading multi object: " << idx << "..." << std::endl;
					auto ptr = leaf_factory->make();
					ptr->parse(inf);
					multi_sections[idx] = ptr;
				}
				break;

				case cur_type_t::batch:
				{
					auto ptr = leaf_factory->make();
					ptr->parse(inf);
					multi_sections[++idx] = ptr;
				}
				break;

				case cur_type_t::super:
					super_itr->second->parse(inf);
					break;

				case cur_type_t::leaf:
					leaf_itr->second->parse(inf);
					break;

				default:
					throw "Impossible";
					break;
			}

		}
		std::cerr << "Aborted on reading: " << s << ":" << std::endl
			<< " - `" << s << "' is no known super section or leaf" << std::endl;
		if(type == type_t::multi)
		{
			std::cerr << " - `" << s << "' is no number" << std::endl;
		}
		else if(type == type_t::batch)
		{
			std::cerr << " - `" << s << "' does not match the batch string `" << batch_str << "'" << std::endl;
		}
		std::cerr << std::endl;
	}

	supersection_t(type_t type = type_t::sections, bool required = true) :
		type(type),
		required(required)
		{}
};

class scene_grids_t : public supersection_t
{
public:
	scene_grids_t() : supersection_t(type_t::multi) {
		init_factory<leaf_template_t<grid_t>>();
	}
};

class scene_path_t : public supersection_t
{
public:
	scene_path_t() : supersection_t(type_t::multi) {
		init_factory<leaf_template_t<path_node>>();
	}
};


class scene_2_t : public supersection_t // TODO: public?
{
public:
	scene_2_t() : supersection_t(type_t::batch)
	{
		init_leaf<leaf_template_t<std::string>>("description");
		init_leaf<leaf_template_t<n_t>>("n");
		init_subsection<scene_grids_t>("grids");

		init_factory<scene_path_t>();
		set_batch_str("path");
	}
};

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

	void parse(infile_t& inf)
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

		infile_t inf;
/*		scene_t scene;

		try {
			scene.parse(inf);
		} catch(infile_t::error_t ife) {
			std::cout << "infile line " << ife.line << ": "	 << ife.msg << std::endl;
		}

		scene.dump(std::cout);*/
		scene_2_t scene;
		try {
			scene.parse(inf);
		} catch(infile_t::error_t ife) {
			std::cout << "infile line " << ife.line << ": "	 << ife.msg << std::endl;
		}

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

