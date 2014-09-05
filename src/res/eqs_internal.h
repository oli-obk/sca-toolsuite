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

#ifndef EQS_INTERNAL_H
#define EQS_INTERNAL_H

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi.hpp>

#include "equation_solver.h"
#include "eqs_functions.h"

namespace ascii = boost::spirit::ascii;
namespace qi = boost::spirit::qi;

namespace eqsolver {

// TODO: generic version? (with specialization for _make_0)
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

const boost::phoenix::function< _make_2<vaddr::var_array<false>> > make_array_index;
const boost::phoenix::function< _make_2<vaddr::var_array<true>> > make_array_addr;
const boost::phoenix::function< _make_1<vaddr::var_helper<true>> > make_helper_index;
const boost::phoenix::function< _make_1<vaddr::var_helper<false>> > make_helper_index_var;
//const boost::phoenix::function< _make_1<vaddr::var_helper> > make_helper_index;
const boost::phoenix::function< _make_0<vaddr::var_x> > make_x;
const boost::phoenix::function< _make_0<vaddr::var_y> > make_y;


template <typename Iterator>
struct str_int_t : qi::grammar<Iterator, expression_ast(), ascii::space_type>
{
	str_int_t() : str_int_t::base_type(str_int)
	{
		str_int = (qi::char_("-") >> *qi::char_("0-9")) | (*qi::char_("0-9"));
	}
	qi::rule<Iterator, std::string()> str_int;
};


template <typename Iterator, bool>
struct var_calculator : qi::grammar<Iterator, expression_ast(), ascii::space_type>
{
	var_calculator() : var_calculator::base_type(var_base)
	{
		const std::string null_str = "0";
		using qi::_val;
		using qi::_1;
		using qi::_2;

		str_int =  (qi::char_("-") >> *qi::char_("0-9")) | (*qi::char_("0-9"));
		helper_variable = "h[" >> (str_int [_val = make_helper_index_var(_1)]) >> ']';
		array_variable = "a[" >> ( str_int >> ',' >> str_int ) [_val = make_array_index(_1, _2)] >> ']';

		other = qi::char_("x") [ _val = make_x() ]
			| qi::char_("y") [ _val = make_y() ]
			| qi::char_("v") [ _val = make_array_index(null_str, null_str)];

		var_base = helper_variable | array_variable | other;
	}
	qi::rule<Iterator, std::string()> str_int;
	qi::rule<Iterator, expression_ast(), ascii::space_type> var_base;
	qi::rule<Iterator, vaddr()> array_variable, helper_variable, other;
};

// TODO: unite with above class
template <typename Iterator>
struct var_calculator<Iterator, true> : qi::grammar<Iterator, expression_ast(), ascii::space_type>
{
	var_calculator() : var_calculator::base_type(var_base)
	{
		using qi::_val;
		using qi::_1;
		using qi::_2;
		boost::phoenix::function<expression_ast::factory_f<expression_ast> > make_expr;

		str_int =  (qi::char_("-") >> *qi::char_("0-9")) | (*qi::char_("0-9"));

		helper_address = "h[" >> (str_int [_val = make_helper_index(_1)]) >> ']';
		array_address = "a[" >> ( str_int >> ',' >> str_int ) [_val = make_array_addr(_1, _2)] >> ']';

		var_base = helper_address [ _val = make_expr(_1) ]
			| array_address [ _val = make_expr(_1) ]; // TODO: [] necessary?
	}
	qi::rule<Iterator, std::string()> str_int;
	qi::rule<Iterator, expression_ast(), ascii::space_type> var_base;
	qi::rule<Iterator, vaddr()> helper_address, array_address;
};


//! calculator grammar to build expression trees
template <typename Iterator>
struct calculator : qi::grammar<Iterator, expression_ast(), ascii::space_type>
{
	/*
		We use an order like in the C language
	*/
	calculator() : calculator::base_type(comma_assignment)
	{
		using qi::_val;
		using qi::_1;
		using qi::_2;
		using qi::uint_;
		boost::phoenix::function<expression_ast::factory_f<expression_ast> > make_expr;

		comma_assignment = assignment [_val = _1]
				>> *( (',' >> assignment [_val = com_func(_val, _1)]));

		assignment = (variable_address [_val = make_expr(_1)] >> ":="
			>> tern_expression [_val = ass_func(_val,_1)]) | tern_expression [_val = _1];

		tern_expression = or_expression [_val = _1]
			>> *("?" >> or_expression >> ":" >> or_expression) [_val = tern_func(_val, _1, _2)];

		or_expression = and_expression [_val = _1]
			>> *("||" >> and_expression [_val = _val || _1]);

		and_expression = lor [_val = _1]
			>> *("&&" >> lor [_val = _val && _1]);

		lor = lxor[_val = _1] >> *("|" >> lxor [_val = _val | _1]);
		lxor = land[_val = _1] >> *("^" >> land [_val = _val ^ _1]);
		land = equation [_val = _1] >> *("&" >> equation [_val = _val & _1]);

		equation =
			inequation [_val = _1]
			>> *( ("==" >> inequation [_val == _1])
			    | ("!=" >> inequation [_val != _1])
			    )
			;

		inequation =
			shift [_val = _1]
			>> *( ('<' >> shift [_val < _1])
			    | ('>' >> shift [_val > _1])
			    | ("<=" >> shift [_val <= _1])
			    | (">=" >> shift [_val >= _1])
			    )
			;

		shift = sum [_val = _1]
			>> *(   ("<<" >> sum            [_val << _1])
			    |   (">>" >> sum            [_val >> _1])
			    )
			;

		sum =
			term                            [_val = _1]
			>> *(   ('+' >> term            [_val += _1])
			    |   ('-' >> term            [_val -= _1])
			    )
			;

		term =
			factor                          [_val = _1]
			>> *(   ('*' >> factor          [_val *= _1])
			    |   ('/' >> factor          [_val /= _1])
			    |   ('%' >> factor          [_val %= _1])
			    )
			;

		function =
			( "min(" >> assignment [_val = _1] >> ','
				>> assignment [_val = min_func(_val,_1)] >> ')')
			| ( "max(" >> assignment [_val = _1] >> ','
				>> assignment [_val = max_func(_val,_1)] >> ')')
			| ( "abs(" >> assignment [_val = abs_func(_1)] >> ')')
			| ( "sqrt(" >> assignment [_val = sqrt_func(_1)] >> ')');

		factor =
			variable                        [_val = make_expr(_1)]
			| uint_                         [_val = make_expr(_1)]
			|   '(' >> comma_assignment           [_val = make_expr(_1)] >> ')'
			|   ('-' >> factor              [_val = neg(_1)])
			|   ('+' >> factor              [_val = _1])
			|   ('!' >> factor              [_val = not_func(_1)])
			| function                  [_val = _1]
			;

	}
	qi::rule<Iterator, expression_ast(), ascii::space_type> comma_assignment, assignment,
	tern_expression,
	or_expression, and_expression, equation, inequation, sum, term, factor, function,
	lor, lxor, land, shift;
	var_calculator<Iterator, false> variable;
	var_calculator<Iterator, true> variable_address;
};

}

#endif // EQS_INTERNAL_H
