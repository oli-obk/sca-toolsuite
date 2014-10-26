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
#include "util.h"
#include "grid.h" // TODO: avoid this!
#include "ca_basics.h"

namespace sca { namespace io {

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
	constexpr static std::size_t READ_BUF_SIZE = 256;
	char read_buffer[READ_BUF_SIZE] = { 0 };
	std::istream& stream;
	bool good = true;
public:
	operator bool() { return good; }
	void set_bad() { good = false; }

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
	std::string read_string_newline();
	section_t read_section();
};

template<class T> class leaf_template_t;

class leaf_base_t
{
private:
	bool read = false;
public:
	bool is_read() const { return read; }
	void set_read() { read = true; }
	void _parse(secfile_t& inf) {
		set_read();
		parse(inf);
	}
	virtual void parse(secfile_t& inf) = 0;
	virtual void dump(std::ostream& stream) const = 0;
	void clear() { read = false; }
	friend std::ostream& operator<<(std::ostream& stream, const leaf_base_t& l);
	template<class T>
	const leaf_template_t<T>& as() { return dynamic_cast<
		const leaf_template_t<T>&>(*this); }
//	virtual const leaf_base_t& cast() const = 0;
};

inline secfile_t& operator>>(secfile_t& s, leaf_base_t& l)
{
	l.parse(s);
	return s;
}

template<class T>
class leaf_template_t : public leaf_base_t
{
	T t;

	template<class T2> class knowledge {};

	template<class T2>
	constexpr static bool read_newline(const knowledge<T2>&) { return true; }
	template<class... Args>
	constexpr static bool read_newline(const knowledge<_grid_t<Args...>>&) { return false; }
	template<class... Args>
	constexpr static bool read_newline(const knowledge<ca::_n_t<Args...>>&) { return false; }

public:
	void parse(secfile_t& inf) { inf.stream >> t; std::cerr << "Read object via cin: " << t << std::endl;
		const knowledge<T> k{};
		if(read_newline(k)) {
			inf.read_newline();
			inf.read_newline();
		}
	}
	void dump(std::ostream& stream) const { stream << t; } /* TODO: might need 1 or 2 newlines */
	operator T&() noexcept { return t; }
	operator const T&() const noexcept { return t; }
	const T& value() const noexcept { return t; }
	T& value() noexcept { return t; }

	const T& value(const T& def) const noexcept { return is_read() ? t : def; }
	T& value(T& def) noexcept { return is_read() ? t : def; }

	template<class ...A> leaf_template_t(const A&... a) : t(a...) {
	}
};

// TODO: not sure why this is needed:
template<>
class leaf_template_t<std::string> : public leaf_base_t
{
	using T = std::string;
	T t;
public:
	void parse(secfile_t& inf);
	void dump(std::ostream& stream) const { stream << t; }
	const T& value() const noexcept { return t; }
	T& value() noexcept { return t; }
	const T& value(const T& def) const noexcept { return is_read() ? t : def; }
	T& value(T& def) noexcept { return is_read() ? t : def; }
};

/*
template<>
class leaf_template_t<grid_t> : public leaf_base_t
{
	using T = grid_t;
	T t;
public:
	void parse(secfile_t& inf);
	void dump(std::ostream& stream) const { stream << t; }
	operator const T&() const noexcept { return t; }
	const T& value() const noexcept { return t; }
	T& value() noexcept { return t; }
	template<class ...A> leaf_template_t(A... a) : t(a...) {}
};*/

template<>
class leaf_template_t<void> : public leaf_base_t
{
public:
	void parse(secfile_t& ) noexcept {}
	void dump(std::ostream& ) const noexcept {}
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

	typedef void (supersection_t::* hook_func_t)(void);

private:
	const bool required;
	std::string batch_str;

	class factory_base
	{
	public:
		virtual leaf_base_t* make() const = 0;
	};

	template<class T, class ...Args>
	class factory : public factory_base
	{
		const std::tuple<const Args& ...> args;
		template<std::size_t... Ids>
		T* _make(util::seq<Ids...>) const {
			return new T(std::get<Ids>(args)...);
		}
	public:
		factory(const Args&... args) : args(args...) {}
		virtual T* make() const { return _make(util::make_seq<sizeof...(Args)>{}); }
	};

	factory_base* leaf_factory;


	std::map<std::string, leaf_base_t*> supersections; // TODO: map of supersection_t?
	std::map<std::size_t, leaf_base_t*> multi_sections;
	std::map<std::string, leaf_base_t*> leafs;

	std::map<leaf_base_t*, hook_func_t> hooks;

	bool check_required()
	{
		(void)required; // TODO
		return true;
	}

protected:

	template<class T, class ...Args>
	void init_subsection(const char* sec_name, const Args& ...args) {
		supersections[sec_name] = new T(args...);
	}
	template<class T, class ...Args>
	void init_subsection_cb(const char* sec_name, const hook_func_t& on_finished, const Args& ...args) {
		init_subsection<T, Args...>(sec_name, args...); // TODO: use ret val
		hooks.emplace(supersections[sec_name], on_finished);
	}

	template<class T, class ...Args>
	void init_leaf(const char* leaf_name, const Args& ...args) {
		leafs[leaf_name] = new T(args...);
	}
	template<class T, class ...Args>
	void init_leaf_cb(const char* leaf_name, const hook_func_t& on_finished, const Args& ...args) {
		init_leaf<T, Args...>(leaf_name, args...); // TODO: use ret val
		hooks.emplace(leafs[leaf_name], on_finished);
	}

	template<class T, class ...Args>
	void init_factory(const Args&... args) {
		leaf_factory = new factory<T, Args...>(args...);
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

	template<class M, class K>
	static leaf_base_t& save_map_find(const M& map, const K& key)
	{
		auto itr = map.find(key);
		if((itr) == map.end()) {
			std::cerr << "Looking up: " << key << std::endl;
			throw "Key not found";
		}
		else
		 return *(itr->second);
	}

	const leaf_base_t& numbered(std::size_t id) const {
		return save_map_find(multi_sections, id);
	}

	leaf_base_t& numbered(std::size_t id) {
		return save_map_find(multi_sections, id);
	}

	template<class Res, class T> static Res m_cast(T& t) {
	//	std::cerr << "cast: " << typeid(T).name() << " to " << typeid(Res).name() << std::endl;
		return dynamic_cast<Res>(t);
	}

	template<class Res, class T> static const Res m_cast(const T& t) {
	//	std::cerr << "cast: " << typeid(T).name() << " to " << typeid(Res).name() << std::endl;
		return dynamic_cast<Res>(t);
	}

	virtual void process() {}

public:
	void clear() {
		leaf_base_t::clear();
		multi_sections.clear();
	}

	//! multi-container size
	std::size_t max() const { return multi_sections.size(); }

	//! named supersection
	const supersection_t& operator[](const char* name) const {
		return m_cast<const supersection_t&>(
			save_map_find(supersections, name));
	}

	//! named supersection
	supersection_t& operator[](const char* name) {
		return m_cast<supersection_t&>(
			save_map_find(supersections, name));
	}

	//! numbered supersection
	const supersection_t& operator[](std::size_t id) const {
		return m_cast<const supersection_t&>(numbered(id));
	}

	//! numbered supersection
	supersection_t& operator[](std::size_t id) {
		return m_cast<supersection_t&>(numbered(id));
	}

	//! access for the leaf itself. probably useless
	template<class T>
	leaf_template_t<T>& leaf(const char* name) {
		return m_cast<leaf_template_t<T>&>(
			save_map_find(leafs, name));
	}

	//! access for the leaf itself. probably useless
	template<class T> // TODO: -> private
	const leaf_template_t<T>& leaf(const char* name) const {
		return m_cast<const leaf_template_t<T>&>(
			save_map_find(leafs, name));
	}

	//! named value
	template<class T>
	const T& value(const char* name) const {
		return leaf<T>(name).value();
	}

	//! named value
	template<class T>
	T& value(const char* name) {
		return leaf<T>(name).value();
	}

	//! numbered value
	template<class T>
	const T& value(std::size_t id) const {
		return m_cast<const leaf_template_t<T>&>(numbered(id)).value();
	}

	//! numbered value
	template<class T>
	T& value(std::size_t id) {
		return m_cast<leaf_template_t<T>&>(numbered(id)).value();
	}

	//! named value + default
	template<class T>
	const T& value(const char* name, const T& def) const {
		return leaf<T>(name).value(def);
	}

	//! named value + default
	template<class T>
	T& value(const char* name, T& def) {
		return leaf<T>(name).value(def);
	}

	//! numbered value + default
	template<class T>
	const T& value(std::size_t id, const T& def) const {
		return m_cast<const leaf_template_t<T>&>(numbered(id)).value(def);
	}

	//! numbered value + default
	template<class T>
	T& value(std::size_t id, T& def) {
		return m_cast<leaf_template_t<T>&>(numbered(id)).value(def);
	}

	template<class KeyT>
	using map_itr = typename std::map<KeyT, leaf_base_t*>::const_iterator;

	template<class T, class KeyT>
	class val_pair
	{
		map_itr<KeyT> itr;
	public:
	//	val_pair(const map_itr<KeyT>& itr) {}
		const KeyT& key() const { return itr->first; }
		const T& value() const { return dynamic_cast<leaf_template_t<T>*>(itr->second)->value(); }
		val_pair(map_itr<KeyT> itr) : itr(itr) {}
	};

	template<class T, class KeyT>
	class val_itr
	{
		map_itr<KeyT> itr;
	public:
		val_itr& operator++() { return ++itr, *this; }
		val_pair<T, KeyT> operator*() const { return {itr}; }
		bool operator!=(const val_itr& other) { return itr != other.itr; }
		val_itr(const map_itr<KeyT>& itr) : itr(itr) {}
	};

	template<class T, class KeyT>
	class val_cont
	{
		const std::map<KeyT, leaf_base_t*>& map;
	public:
		using iterator = val_itr<T, KeyT>;
		iterator begin() { return { map.cbegin() }; }
		iterator end() { return { map.cend() }; }
		iterator find(const KeyT& key) { return { map.find(key) }; }
		val_cont(const std::map<KeyT, leaf_base_t*>& map) : map(map) {}
	};


	template<class T>
	val_cont<T, std::string> named_values() const {
		return { supersections }; }

	template<class T>
	val_cont<T, std::size_t> numbered_values() const {
		return { multi_sections }; }




/*	class container
	{
		const

	};*/


	/*template<class T>
	const leaf_template_t<T>& num(const char* name) const {
		return dynamic_cast<const leaf_template_t<T>&>(
			save_map_find(leafs, name));
	}*/



	// TODO: itr class


	void dump(std::ostream& stream) const;

	void parse(secfile_t& inf);

	supersection_t(type_t type = type_t::sections, bool required = true) :
		type(type),
		required(required)
		{}
};

}}

#endif // SECFILE_H
