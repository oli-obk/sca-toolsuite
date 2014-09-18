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

#ifndef SECFILE_H
#define SECFILE_H

#include <string>
#include <iostream>
#include <map>

namespace io {

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

struct secfile_t
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

	error_t mk_error(const char* err) const;

	secfile_t(std::istream& stream = std::cin) :
		stream(stream) {}

	void read_newline();

	static bool m_atoi(int& res, const char* str);

	bool read_int(int& i);

	const char* get_next_line();

	void clear_buffer() { *read_buffer = 0; }

	std::string read_string_noclear();

	std::string read_string();

	bool match_string(const char* str);

	/*void read_grid(grid_t& grid) {
		stream >> grid;
	}*/

	section_t read_section();
};

class leaf_base_t
{
public:
	virtual void parse(secfile_t& inf) = 0;
	virtual void dump(std::ostream& stream) const = 0;
	friend std::ostream& operator<<(std::ostream& stream, const leaf_base_t& l);
};

template<class T>
class leaf_template_t : public leaf_base_t
{
	T t;
public:
	void parse(secfile_t& inf) { inf.stream >> t; std::cerr << "Read object via cin: " << t << std::endl; }
	void dump(std::ostream& stream) const { stream << t; }
};

template<>
class leaf_template_t<std::string> : public leaf_base_t
{
	std::string t;
public:
	void parse(secfile_t& inf);
	void dump(std::ostream& stream) const { stream << t; }
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

	bool check_required()
	{
		(void)required; // TODO
		return true;
	}

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

	cur_type_t check_string(secfile_t& inf, std::string& s);


public:

	void dump(std::ostream& stream) const;

	void parse (secfile_t& inf);

	supersection_t(type_t type = type_t::sections, bool required = true) :
		type(type),
		required(required)
		{}
};

}

#endif // SECFILE_H
