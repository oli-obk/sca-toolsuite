/*************************************************************************/
/* sca toolsuite - a toolsuite to simulate sandpile cellular automata.   */
/* Copyright (C) 2011-2012                                               */
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
#include "equation_solver.h"

#include <iostream>
#include <vector>
#include <string>

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

// we are usually against macros. I think it is okay to make one here, still!
#define MAKE_OP(OP, VISUAL, EXECUTE) \
	expression_ast& expression_ast::operator OP(expression_ast const& rhs) \
	{ \
		expr = binary_op<int, int, int>(VISUAL, &EXECUTE, expr, rhs); \
		return *this; \
	}

namespace eqsolver // TODO: everything should be in this namespace?!
{
	// static variable definitions
/*	int variable_print::width;
	variable_print::result_type variable_print::x, variable_print::y;
	const variable_print::result_type *variable_print::v;
	int* variable_print::helper_vars;
*/
	MAKE_OP(+=,'+',f2i_add);
	MAKE_OP(-=,'-',f2i_sub);
	MAKE_OP(*=,'*',f2i_mul);
	MAKE_OP(/=,'/',f2i_div);
	MAKE_OP(%=,'%',f2i_mod);
	MAKE_OP(<,'<',f2i_lt);
	MAKE_OP(>,'>',f2i_gt);
	MAKE_OP(<=,'l',f2i_le);
	MAKE_OP(>=,'g',f2i_ge);
	MAKE_OP(==,'=',f2i_eq);
	MAKE_OP(!=,'!',f2i_neq);
	MAKE_OP(&&,'&',f2i_and);
	MAKE_OP(||,'|',f2i_or);
}

void build_tree_from_equation(const char* equation, eqsolver::expression_ast* ast)
{
	using boost::spirit::ascii::space;

	typedef std::string::const_iterator iterator_type;
	typedef eqsolver::calculator<iterator_type> calculator;
	calculator calc; // our grammar

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

int solve_single_equation(const char* equation, int height, int width, int x, int y, int* v)
{
/*	eqsolver::expression_ast ast;
	build_tree_from_equation(equation, &ast);

	//eqsolver::ast_print solver(height, width, x,y,v);

	return solver(ast);*/
	exit(99); // TODO!
}
