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

#ifndef EQS_FUNCTIONS_H
#define EQS_FUNCTIONS_H

#include <boost/spirit/include/support_info.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace eqsolver
{

template<typename _result_type>
struct _make_0
{
	typedef _result_type result_type;
	inline result_type operator()() const {
		return result_type();
	}
};
template<typename _result_type>
struct _make_1
{
	template <typename>
	struct result { typedef _result_type type; };

	template <typename T>
	inline typename result<T>::type operator()(T const& c1) const {
		return _result_type(c1);
	}
};

template<typename _result_type>
struct _make_2
{
	template <typename, typename>
	struct result { typedef _result_type type; };

	template <typename T1, typename T2>
	inline typename result<T1, T2>::type operator()
		(T1 const& c1, T2 const& c2) const {
		return _result_type(c1, c2);
	}
};

class expression_ast;

/*
	Again, I think these macros are okay here.
*/
#define MAKE_UNARY_FUNC(OP) struct OP \
{ \
	template <typename T> \
	struct result { typedef T type; }; \
	expression_ast operator()(expression_ast const& expr) const; \
}; \
const boost::phoenix::function<OP> OP;

#define MAKE_BINARY_FUNC(OP) struct OP \
{ \
	template <typename T1, typename T2> \
	struct result { typedef T2 type; }; \
	expression_ast operator()(expression_ast const& expr1, expression_ast const& expr2) const; \
}; \
const boost::phoenix::function<OP> OP;

#define MAKE_BINARY_FUNC_ADDR(OP) struct OP \
{ \
	template <typename T1, typename T2> \
	struct result { typedef T2 type; }; \
	expression_ast operator()(expression_ast const& expr1, expression_ast const& expr2) const; \
}; \
const boost::phoenix::function<OP> OP;

#define MAKE_TERNARY_FUNC(OP) struct OP \
{ \
	template <typename T1, typename T2, typename T3> \
	struct result { typedef T3 type; }; \
	expression_ast operator()(expression_ast const& expr1, expression_ast const& expr2, \
		expression_ast const& expr3) const; \
}; \
const boost::phoenix::function<OP> OP;

/*
* ^^ TODO: try to use functions instead of macros
*/

// We should be using expression_ast::operator-. There's a bug
// in phoenix type deduction mechanism that prevents us from
// doing so. Phoenix will be switching to BOOST_TYPEOF. In the
// meantime, we will use a phoenix::function below:
MAKE_UNARY_FUNC(neg);
MAKE_UNARY_FUNC(not_func);
MAKE_UNARY_FUNC(abs_func);
MAKE_UNARY_FUNC(sqrt_func);
MAKE_UNARY_FUNC(rand_func);
MAKE_BINARY_FUNC(min_func);
MAKE_BINARY_FUNC(max_func);
MAKE_BINARY_FUNC_ADDR(ass_func);
MAKE_BINARY_FUNC(com_func);
MAKE_TERNARY_FUNC(tern_func);

}

#endif // EQS_FUNCTIONS_H
