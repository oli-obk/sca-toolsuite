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

#include <cctype>
#include <cstring>

#include "secfile.h"

namespace sca { namespace io {

bool is_number_digit(const char& c) noexcept
{
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


supersection_t::cur_type_t supersection_t::check_string(secfile_t &inf, std::string &s)
{
	s = inf.read_string_noclear();
	cur_type_t res;
	if(type == type_t::multi && is_number(s.c_str()))
	{
		std::cerr << "Found multi object: `" << s << "'" << std::endl;
		res = cur_type_t::multi;
	}
	else if((type == type_t::batch) && (batch_str == s))
	{
		std::cerr << "Found batch string: `" << s << "'" << std::endl;
		res = cur_type_t::batch;
	}
	else if((super_itr = supersections.find(s)) != supersections.end())
	{
		std::cerr << "Found supersection: `" << s << "'" << std::endl;
		res = cur_type_t::super;
	}
	else if((leaf_itr = leafs.find(s)) != leafs.end()) // TODO: store itr in mutable class var?
	{
		std::cerr << "Found leaf: `" << s << "'" << std::endl;
		res = cur_type_t::leaf;
	}
	else {
		std::cerr << "No match: `" << s << "'" << std::endl;
		res = cur_type_t::unknown;
	}

	if(res != cur_type_t::unknown)
	{
		inf.clear_buffer();
		inf.read_newline();
	}

	return res;
}

void supersection_t::dump(std::ostream &stream) const
{
	for(const auto& pr : supersections)
	{
		stream << pr.first << std::endl << std::endl
			<< (*pr.second);
	}

	for(const auto& pr : leafs)
	{
		stream << pr.first << std::endl << std::endl
			<< (*pr.second);
	}

	for(const auto& pr : multi_sections)
	{
		stream << pr.first << std::endl << std::endl
			<< (*pr.second);
	}
}

void supersection_t::parse(secfile_t &inf)
{
	std::string s;
	cur_type_t cur;

	int idx = -1; // TODO: size_t

	while(cur_type_t::unknown != (cur = check_string(inf, s))) // TODO: while type = inf.read_string_no_clear() ...
	{
		std::cerr << "Trying to parse section: " << s << std::endl;

		switch(cur)
		{
		case cur_type_t::multi:
		{
			idx = std::stoi(s);
			std::cerr << "Reading multi object: " << idx << "..." << std::endl;
			auto ptr = leaf_factory->make();
			ptr->_parse(inf);
			multi_sections[idx] = ptr;
		}
		break;

		case cur_type_t::batch:
		{
			auto ptr = leaf_factory->make();
			ptr->_parse(inf);
			multi_sections[++idx] = ptr;
		}
		break;

		case cur_type_t::super:
			super_itr->second->_parse(inf);
			break;

		case cur_type_t::leaf:
			leaf_itr->second->_parse(inf);
			break;

		default:
			throw "Impossible";
			break;
		}

	}
	std::cerr << "Left on reading: " << s << ":" << std::endl
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

	if(!check_required() || inf.stream.eof())
	 inf.set_bad();
}


void leaf_template_t<std::string>::parse(secfile_t &inf) { t = inf.read_string_newline();  std::cerr << "Read string: " << t << std::endl; }


std::ostream &operator<<(std::ostream &stream, const leaf_base_t &l) {
	return l.dump(stream), stream;
}


secfile_t::error_t secfile_t::mk_error(const char *err) const {
	std::cerr << "parsing stopped after: " << read_buffer << std::endl;
	return error_t { line, err };
}

void secfile_t::read_newline()
{
	if(*get_next_line()) {
		throw mk_error("Expected newline");
	}
	++line;
}

bool secfile_t::m_atoi(int &res, const char *str) {
	return is_number_digit(*str)
		? res = atoi(str), true
		: false;
}

bool secfile_t::read_int(int &i)
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

const char *secfile_t::get_next_line()
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

std::string secfile_t::read_string_noclear()
{
	/*if(!*read_buffer) {
			throw mk_error("Expected a non-empty string");
		}*/
	return ++line, std::string(get_next_line());
}

std::string secfile_t::read_string()
{
	/*if(!*read_buffer) {
			throw mk_error("Expected a non-empty string");
		}*/
	std::string result = read_string_noclear();
	clear_buffer();
	return result;
}

bool secfile_t::match_string(const char *str)
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

std::string secfile_t::read_string_newline()
{
	std::string res = read_string_noclear();
	//if(res.empty())
	//throw mk_error("Expected section, got empty line!");
	if(!stream.good()) {
		std::cerr << "WARNING: stream not good..." << std::endl;
		res = "";
	}
	if(res.size()) // i.e. this was a section
	{
		clear_buffer();
		read_newline();
	}
	return std::move(res);
}

section_t secfile_t::read_section()
{
	return std::move(read_string_newline());
}

}}
