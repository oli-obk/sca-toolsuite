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

#ifndef SERIAL_H
#define SERIAL_H

#include <tuple>
#include <set>
#include <vector>
#include <ostream>
#include <typeinfo>

namespace sca { namespace io {

#if 0
class serializer
{
	std::ostream& stream;

	template<class T>
	serializer& primitive(const T& x) {
		std::cerr << typeid(T).name() << std::endl;
		std::cerr << sizeof(T) << std::endl;
		stream.write((char*)&x, sizeof(T));
		return *this;
	}

public:

/*	serializer(const char* filename) :
		stream(std::ofstream(filename, std::ofstream::binary))
	{
	}*/

	serializer(std::ostream& stream) :
		stream(stream)
	{
	}

	// TODO: friend functions do not belong here
	friend serializer& operator<<(serializer& s, uint8_t x) {
		return s.primitive(x); }
	friend serializer& operator<<(serializer& s, int8_t x) {
		return s.primitive(x); }
	friend serializer& operator<<(serializer& s, uint16_t x) {
		return s.primitive(x); }
	friend serializer& operator<<(serializer& s, int16_t x) {
		return s.primitive(x); }
	friend serializer& operator<<(serializer& s, uint32_t x) {
		return s.primitive(x); }
	friend serializer& operator<<(serializer& s, int32_t x) {
		std::cout << "???" << std::endl;
		return s.primitive(x); }
	friend serializer& operator<<(serializer& s, uint64_t x) {
		return s.primitive(x); }
	friend serializer& operator<<(serializer& s, int64_t x) {
		return s.primitive(x); }



	template<class T>
	friend serializer& operator<<(serializer& s, const std::set<T>& x) {
		s << x.size();
		for(const T& elem : x)
		 operator<<(s, elem);
		return s;
	}

	template<class T>
	friend serializer& operator<<(serializer& s, const std::vector<T>& x) {
		s << x.size();
		for(const T& elem : x)
		 operator<<(s, elem);
		return s;
	}
};

namespace detail
{

template<std::size_t... Is> struct seq {};
template<std::size_t N, std::size_t... Is> struct make_seq : make_seq<N-1, N-1, Is...> {};
template<std::size_t... Is> struct make_seq<0, Is...> : seq<Is...> {};

template<class T>
serializer& dispatch(const T& , serializer& s) {
	return s;
}

template<class T, std::size_t First, std::size_t... Others>
serializer& dispatch(const T& t, serializer& s) {
	std::cerr << "F:" << typeid(decltype(std::get<First>(t))).name() << std::endl;
	return dispatch<T, Others...>(t, s << std::get<First>(t));
} // TODO: member of serializer?

}

template<class ...Members>
class serial // <- the power of C++11
{
	std::tuple<const Members&...> members;
	using self_t = serial<Members...>;

	template<std::size_t... Numbers>
	serializer& dispatch(serializer&s, detail::seq<Numbers...>) const {
		return detail::dispatch<decltype(members),
			Numbers...>(members, s);
	}

public:
	serial(const Members&... members) : members(members...)
	{
	}

	friend serializer& operator<<(serializer& s, const self_t& x) {
		return x.dispatch(s, detail::make_seq<sizeof...(Members)>()); }

};
#endif

#include <type_traits>

class serializer_base
{
public:
	serializer_base(const serializer_base&) = delete;
	serializer_base() = default;
};

class serializer : serializer_base
{
	std::ostream& stream;



public:

/*	serializer(const char* filename) :
		stream(std::ofstream(filename, std::ofstream::binary))
	{
	}*/

	serializer(std::ostream& stream) :
		stream(stream)
	{
	}

	template<class T, typename std::enable_if<
			!std::is_fundamental<T>::value
			&& !std::is_enum<T>::value
		>::type* = nullptr>
	friend serializer& operator<<(serializer& s, const T& x) {
		return s << x; }

	template<class T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
	friend serializer& operator<<(serializer& s, const T& x) {
		return s << (std::size_t)x; }

	template<class T, typename std::enable_if<std::is_fundamental<T>::value>::type* = nullptr>
	friend serializer& operator<<(serializer& s, const T& x) {
		s.stream.write((const char*)&x, sizeof(T));
		return s;
	}
};


class deserializer : serializer_base
{
	std::istream& stream;



public:

/*	deserializer(const char* filename) :
		stream(std::ofstream(filename, std::ifstream::binary))
	{
	}*/

	deserializer(std::istream& stream) :
		stream(stream)
	{
	}

	template<class T,
		typename std::enable_if<
			!std::is_fundamental<T>::value
			&& !std::is_enum<T>::value
			>::type* = nullptr>
	friend deserializer& operator>>(deserializer& s, T& x) {
		return s >> x; }

	template<class T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
	friend deserializer& operator>>(deserializer& s, T& x) {
		std::size_t _x;
		s >> _x;
		x = (T)_x;
		return s;
	}

	template<class T, typename std::enable_if<std::is_fundamental<T>::value>::type* = nullptr>
	friend deserializer& operator>>(deserializer& s, T& x) {
		s.stream.read((char*)&x, sizeof(T));
		return s;
	}

};

template<class T>
serializer& operator<<(serializer& s, const std::set<T>& x) {
	s << (std::size_t)x.size();
	for(const T& elem : x)
	 s << elem;
	return s;
}

template<class T>
deserializer& operator>>(deserializer& s, std::set<T>& x) {
	std::size_t num;
	s >> num;
	for(std::size_t i = 0; i < num; ++i)
	{
		T elem;
		s >> elem;
		x.insert(x.cend(), std::move(elem));
	}
	return s;
}

template<class T>
serializer& operator<<(serializer& s, const std::vector<T>& x) {
	s << (std::size_t)x.size();
	for(const T& elem : x)
	 s << elem; // TODO: serialize all in one!
	return s;
}

template<class T>
deserializer& operator>>(deserializer& s, std::vector<T>& x) {
	std::size_t num;
	s >> num;
	x.reserve(num);
	for(std::size_t i = 0; i < num; ++i)
	{
		T elem;
		s >> elem;
		x.push_back(std::move(elem));
	}
	return s;
}


// TODO: make simple shortenings for tuples?


}}

#endif // SERIAL_H
