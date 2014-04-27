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

#include <string>
#include "eqs_internal.h"

// TODO: not sure which of those we need here, probably none?
/*#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>*/

namespace eqsolver
{

const char* EQUATION_HELP_STR =
"Manipulates input by formula in x, y and value.\n"
"Booleans are treated as integer values 0 and 1.\n"
"Supported Operators by precedence (like in C++):\n"
" 1. Braces: ()\n"
"    Function calls: min(,), max(,), abs(), sqrt()\n"
"    Negation: +, - \n"
"    Not: !\n"
" 2. Multiplicative terms: *, /, %\n"
" 3. Sums: +, -\n"
" 4. inequalities: <, >, <=, >=\n"
" 5. equalities: =, ==, != (\"=\" and \"==\" are both comparisons)"
" 6. and expressions: &&\n"
" 7. or expressions\n";

const char* get_help_description() { return EQUATION_HELP_STR; }

void build_tree(const char* equation, eqsolver::expression_ast* ast)
{
	using boost::spirit::ascii::space;

	typedef std::string::const_iterator iterator_type;
	typedef eqsolver::calculator<iterator_type> calculator;
	const calculator calc; // our grammar

	std::string str = equation;
	//if (str.empty()) // not needed?
	// throw("")
	std::string::const_iterator iter = str.begin();
	std::string::const_iterator end = str.end();

	const bool r = phrase_parse(iter, end, calc, space, *ast);

	if (r && iter == end)
	 return;
	else
	{
		std::string error_msg = "Parsing failed at " + std::string(iter, end);
		throw error_msg;
	}
}

int solve(const char* equation, int height, int width, int x, int y, int* v)
{
/*	eqsolver::expression_ast ast;
	build_tree_from_equation(equation, &ast);

	//eqsolver::ast_print solver(height, width, x,y,v);

	return solver(ast);*/
	(void)equation;
	(void)height;
	(void)width;
	(void)x;
	(void)y;
	(void)v;
	exit(99); // TODO!
}

}

