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

#include <boost/spirit/include/qi.hpp>
#include "equation_solver.h"

namespace ascii = boost::spirit::ascii;
namespace qi = boost::spirit::qi;

namespace eqsolver {

//! calculator grammar to build expression trees
template <typename Iterator>
struct calculator : qi::grammar<Iterator, expression_ast(), ascii::space_type>
{
	/*
		We use an order like in the C language
	*/
	calculator() : calculator::base_type(comma_assignment)
	{
		std::string null_str = "0";
		using qi::_val;
		using qi::_1;
		using qi::_2;
		using qi::uint_;
		using ascii::string;
		boost::phoenix::function<expression_ast::factory_f<expression_ast> > make_expr;

		str_int =  (qi::char_("-") >> *qi::char_("0-9")) | (*qi::char_("0-9"));

		helper_variable = "h[" >> (str_int [_val = make_helper_index_var(_1)]) >> ']';
		helper_address = "h[" >> (str_int [_val = make_helper_index(_1)]) >> ']';
		array_variable = "a[" >> ( str_int >> ',' >> str_int ) [_val = make_array_indexes(_1, _2)] >> ']';

		// fixed indentation:
		/*helper_variable = "h[" >>
		    (
			str_int [_val = make_helper_index(_1)]
		    ) >> ']';
*/
		/*array_variable = qi::char_("a") >> qi::char_("[") >>
		    ( str_int >> ',' >> str_int )
			[_val = make_array_indexes(_1, _2)]
		    >> qi::char_("]");*/




		variable = qi::char_("x") [ _val = make_x() ]
			| qi::char_("y") [ _val = make_y() ]
			| qi::char_("v") [ _val = make_array_indexes(null_str, null_str)];

		//helper_variable = qi::char_("h") >> qi::char_("[") >> str_int >> qi::char_("]");
		//array_variable = qi::char_("a") >> qi::char_("[") >> str_int >> qi::char_(",") >> str_int >> qi::char_("]");




		comma_assignment = assignment [_val = _1]
				>> *( (',' >> assignment [_val = com_func(_val, _1)]));

		assignment = (helper_address [_val = make_expr(_1)] >> ":="
			>> tern_expression [_val = ass_func(_val,_1)]) | tern_expression [_val = _1];

		tern_expression = or_expression [_val = _1]
			>> *("?" >> or_expression >> ":" >> or_expression) [_val = tern_func(_val, _1, _2)];

		or_expression = and_expression [_val = _1]
			>> *("||" >> and_expression [_val = _val || _1]);

		and_expression = equation [_val = _1]
			>> *("&&" >> equation [_val = _val && _1]);

		equation =
			inequation [_val = _1]
			>> *( ("==" >> inequation [_val == _1])
		//	    | ('=' >> inequation [_val == _1])
			    | ("!=" >> inequation [_val != _1])
			    )
			;

		inequation =
			sum [_val = _1]
			>> *( ('<' >> sum [_val < _1])
			    | ('>' >> sum [_val > _1])
			    | ("<=" >> sum [_val <= _1])
			    | (">=" >> sum [_val >= _1])
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

		//variable = +qi::char_("xyvXYV");

		//array_subscript = array_variable [_val = _1]
		//	| helper_variable [_val = _1];


		//array_subscript = ("a[" >> (*qi::char_("0-9")) >> (*qi::char_(",")) >> (*qi::char_("0-9")) >> ']')
		//		| ("A[" >> (*qi::char_("0-9")) >> (*qi::char_(",")) >> (*qi::char_("0-9")) >> ']');

		//("a[" >> (*qi::char_("0-9")) >> (*qi::char_(",")) >> (*qi::char_("0-9")) >> ']')


		function =
			( "min(" >> assignment [_val = _1] >> ','
				>> assignment [_val = min_func(_val,_1)] >> ')')
			| ( "max(" >> assignment [_val = _1] >> ','
				>> assignment [_val = max_func(_val,_1)] >> ')')
			| ( "abs(" >> assignment [_val = abs_func(_1)] >> ')')
			| ( "sqrt(" >> assignment [_val = sqrt_func(_1)] >> ')');

		factor =
			variable                        [_val = make_expr(_1)]
			//| array_subscript               [_val = _1]
			| helper_variable [_val = make_expr(_1)]
			| array_variable  [_val = make_expr(_1)]
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
	or_expression, and_expression, equation, inequation, sum, term, factor, function;
	qi::rule<Iterator, vaddr()> variable, array_variable, helper_variable, helper_address;
	qi::rule<Iterator, std::string()> str_int;
	//qi::rule<Iterator, std::string()> array_variable, helper_variable, variable, array_subscript, str_int;
};

}

#endif // EQS_INTERNAL_H
