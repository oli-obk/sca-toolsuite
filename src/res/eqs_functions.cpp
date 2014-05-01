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

#include "equation_solver.h"
#include "eqs_functions.h"

namespace eqsolver
{

// we are usually against macros. I think it is okay to make one here, still!
#define MAKE_OP(OP, VISUAL, EXECUTE) \
	expression_ast& expression_ast::operator OP(expression_ast const& rhs) \
	{ \
		expr = binary_op<int, int, int>(VISUAL, &EXECUTE, expr, rhs); \
		return *this; \
	}

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

using ternary_op_i = ternary_op<int, int, int, int>;
using binary_op_i = binary_op<int, int, int>;
using unary_op_i = unary_op<int, int>;

#define DEF_UNARY_FUNC(OP, VISUAL, EXECUTE) \
expression_ast OP::operator()(expression_ast const& expr) const { \
	return expression_ast(unary_op_i(VISUAL, EXECUTE, expr)); \
}

#define DEF_BINARY_FUNC(OP, VISUAL, EXECUTE) \
expression_ast OP::operator()(expression_ast const& expr1, expression_ast const& expr2) const { \
	return expression_ast(binary_op_i(VISUAL, EXECUTE, expr1, expr2)); \
}

#define DEF_BINARY_FUNC_ADDR(OP, VISUAL, EXECUTE) \
expression_ast OP::operator()(expression_ast const& expr1, expression_ast const& expr2) const { \
	return expression_ast(binary_op<int, int*, int>(VISUAL, EXECUTE, expr1, expr2)); \
}

#define DEF_TERNARY_FUNC(OP, VISUAL, EXECUTE) \
expression_ast OP::operator()(expression_ast const& expr1, expression_ast const& expr2, \
	expression_ast const& expr3) const{ \
	return expression_ast(ternary_op_i(VISUAL, EXECUTE, expr1, expr2, expr3)); \
}

DEF_UNARY_FUNC(neg, '-', f1i_neg);
DEF_UNARY_FUNC(not_func, '!', f1i_not);
DEF_UNARY_FUNC(abs_func, 'a', f1i_abs);
DEF_UNARY_FUNC(sqrt_func, 's', f1i_sqrt);
DEF_UNARY_FUNC(rand_func, 'r', f1i_rand);
DEF_BINARY_FUNC(min_func, 'm', f2i_min);
DEF_BINARY_FUNC(max_func, 'n', f2i_max);
DEF_BINARY_FUNC_ADDR(ass_func, 'h', f2i_asn);
DEF_BINARY_FUNC(com_func, ',', f2i_com);
DEF_TERNARY_FUNC(tern_func, '?', f3i_tern);

}

