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

#ifndef EQUATION_SOLVER_H
#define EQUATION_SOLVER_H

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

const char* get_help_description();

/*
	Again, I think these macros are okay here.
*/
#define MAKE_UNARY_FUNC(OP, VISUAL, EXECUTE) struct OP \
{ \
	template <typename T> \
	struct result { typedef T type; }; \
	inline expression_ast operator()(expression_ast const& expr) const { \
		return expression_ast(unary_op(VISUAL, &EXECUTE, expr)); \
	} \
}; \
const boost::phoenix::function<OP> OP;

#define MAKE_BINARY_FUNC(OP, VISUAL, EXECUTE) struct OP \
{ \
	template <typename T1, typename T2> \
	struct result { typedef T2 type; }; \
	inline expression_ast operator()(expression_ast const& expr1, expression_ast const& expr2) const { \
		return expression_ast(binary_op(VISUAL, &EXECUTE, expr1, expr2)); \
	} \
}; \
const boost::phoenix::function<OP> OP;
// typedef expression_ast result_type; // TODO: must work with phoenix 3!

//! namespace of the equation solver
namespace eqsolver
{
	inline int f1i_not(int arg1) { return (int)(arg1==0); }
	inline int f1i_neg(int arg1) { return -arg1; }
	inline int f1i_abs(int arg1) { return std::abs(arg1); }
	inline int f1i_sqrt(int arg1) { return (int)std::sqrt((int)arg1); }
	inline int f2i_add(int arg1, int arg2) { return arg1 + arg2; }
	inline int f2i_sub(int arg1, int arg2) { return arg1 - arg2; }
	inline int f2i_mul(int arg1, int arg2) { return arg1 * arg2; }
	inline int f2i_div(int arg1, int arg2) { return arg1 / arg2; }
	inline int f2i_mod(int arg1, int arg2) { return arg1 % arg2; }
	inline int f2i_min(int arg1, int arg2) { return std::min(arg1, arg2); }
	inline int f2i_max(int arg1, int arg2) { return std::max(arg1, arg2); }
	inline int f2i_lt(int arg1, int arg2) { return (int)(arg1<arg2); }
	inline int f2i_gt(int arg1, int arg2) { return (int)(arg1>arg2); }
	inline int f2i_le(int arg1, int arg2) { return (int)(arg1<=arg2); }
	inline int f2i_ge(int arg1, int arg2) { return (int)(arg1>=arg2); }
	inline int f2i_eq(int arg1, int arg2) { return (int)(arg1==arg2); }
	inline int f2i_neq(int arg1, int arg2) { return (int)(arg1!=arg2); }
	inline int f2i_and(int arg1, int arg2) { return (int)(arg1*arg2); }
	inline int f2i_or(int arg1, int arg2) { return (int)((arg1!=0)||(arg2!=0)); }

	typedef int (*fptr_1i)(int arg1);
	typedef int (*fptr_2i)(int arg1, int arg2);

	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	struct binary_op;
	struct unary_op;
	//! empty expression, can't happen!
	struct nil {};

	//! expression tree node
	struct expression_ast
	{
		typedef boost::variant<
			nil
			, unsigned int
			, std::string  // TODO: needed?
			, boost::recursive_wrapper<expression_ast>
			, boost::recursive_wrapper<binary_op>
			, boost::recursive_wrapper<unary_op>
			>
		type;

		expression_ast() : expr(nil()) {}

		template <typename Expr>
		expression_ast(Expr const& expr) : expr(expr) {}

		expression_ast& operator+=(expression_ast const& rhs);
		expression_ast& operator-=(expression_ast const& rhs);

		expression_ast& operator*=(expression_ast const& rhs);
		expression_ast& operator/=(expression_ast const& rhs);
		expression_ast& operator%=(expression_ast const& rhs);

		expression_ast& operator<(expression_ast const & rhs);
		expression_ast& operator>(expression_ast const & rhs);
		expression_ast& operator<=(expression_ast const & rhs);
		expression_ast& operator>=(expression_ast const & rhs);

		expression_ast& operator==(expression_ast const & rhs);
		expression_ast& operator!=(expression_ast const & rhs);

		expression_ast& operator&&(expression_ast const & rhs);
		expression_ast& operator||(expression_ast const & rhs);

		type expr;
	};

	//! expression tree node extension for binary operators
	struct binary_op
	{
		binary_op(
			char op
			, fptr_2i fptr
			, expression_ast const& left
			, expression_ast const& right)
		: op(op), fptr(fptr), left(left), right(right) {}

		char op;
		fptr_2i fptr;
		expression_ast left;
		expression_ast right;
	};

	//! expression tree node extension for unary operators
	struct unary_op
	{
		unary_op(char op, fptr_1i fptr, expression_ast const& subject)
		: op(op), fptr(fptr), subject(subject) {}

		char op;
		fptr_1i fptr;
		expression_ast subject;
	};

	// We should be using expression_ast::operator-. There's a bug
	// in phoenix type deduction mechanism that prevents us from
	// doing so. Phoenix will be switching to BOOST_TYPEOF. In the
	// meantime, we will use a phoenix::function below:
	MAKE_UNARY_FUNC(neg, '-', f1i_neg);
	MAKE_UNARY_FUNC(not_func, '!', f1i_not);
	MAKE_UNARY_FUNC(abs_func, 'a', f1i_abs);
	MAKE_UNARY_FUNC(sqrt_func, 's', f1i_sqrt);
	MAKE_BINARY_FUNC(min_func, 'm', f2i_min);
	MAKE_BINARY_FUNC(max_func, 'n', f2i_max);

	//! Class for iterating an expression tree and printing the result.
	struct ast_print
	{
		// h and w shall be internal, since we want to avoid adding 2
		// so it is still general to non-bordered areas
		int height, width;
		typedef unsigned int result_type;
		const result_type x,y, *v;

		inline int position(int _x, int _y) const { return (_y*(width+1)+_x+1); }

		inline result_type operator()(qi::info::nil) const { return 0; }
		inline result_type operator()(int n) const { return n;  }
		inline result_type operator()(std::string c) const {
			switch(c[0]) {
				case 'x': return x;
				case 'y': return y;
				case 'v': return *v;
				default:
				{
					std::string help_str = c;
					 // TODO: this is waste of memory - make , to \0 instead
					int comma_pos = help_str.find(',');
					help_str[comma_pos]=0;
					int xoff = atoi(help_str.c_str());
					help_str=c.substr(comma_pos+1);
					int yoff = atoi(help_str.c_str());
					return v[xoff+(yoff*width)];
				}
			}
		}

		inline result_type operator()(expression_ast const& ast) const {
			return boost::apply_visitor(*this, ast.expr);
		}

		inline result_type operator()(binary_op const& expr) const
		{
			return expr.fptr(
				boost::apply_visitor(*this, expr.left.expr),
				boost::apply_visitor(*this, expr.right.expr)
			);
		}

		inline result_type operator()(unary_op const& expr) const {
			return expr.fptr(boost::apply_visitor(*this, expr.subject.expr));
		}
		ast_print(int _x) : x(_x), y(0), v(NULL) {}
		ast_print(int _height, int _width, int _x, int _y, const int* _v)
			: height(_height), width(_width), x(_x), y(_y), v((const result_type*)_v) {
		}
	//	ast_print(int x, int y, int* v) : x(x), y(y), v((result_type*)v) {}

	};

	//! Class for iterating an expression tree and print the used area in the array.
	//! The result is an int describing the half size of a square.
	struct ast_area
	{
		typedef unsigned int result_type;

		inline result_type operator()(qi::info::nil) const { return 0; }
		inline result_type operator()(int n) const { return 0;  }
		inline result_type operator()(std::string c) const {
			switch(c[0]) {
				case 'x': case 'y': case 'v': return 0;
				default:
				{
					std::string help_str = c;
					 // TODO: this is waste of memory - make , to \0 instead
					int comma_pos = help_str.find(',');
					help_str[comma_pos]=0;
					int xoff = atoi(help_str.c_str());
					help_str=c.substr(comma_pos+1);
					int yoff = atoi(help_str.c_str());
					return std::max(xoff,yoff);
				}
			}
		}

		inline result_type operator()(expression_ast const& ast) const {
			return boost::apply_visitor(*this, ast.expr);
		}

		inline result_type operator()(binary_op const& expr) const
		{
			return std::max(
				boost::apply_visitor(*this, expr.left.expr),
				boost::apply_visitor(*this, expr.right.expr)
			);
		}

		inline result_type operator()(unary_op const& expr) const {
			return boost::apply_visitor(*this, expr.subject.expr);
		}

	};

	//! caluclator grammar to build expression trees
	template <typename Iterator>
	struct calculator : qi::grammar<Iterator, expression_ast(), ascii::space_type>
	{
		/*
			We use an order like in the C language
		*/
		calculator() : calculator::base_type(expression)
		{
			using qi::_val;
			using qi::_1;
			using qi::uint_;
			using ascii::string;

			expression = and_expression [_val = _1]
				>> *("||" >> and_expression [_val = _val || _1]);

			and_expression = equation [_val = _1]
				>> *("&&" >> equation [_val = _val && _1]);

			equation =
				inequation [_val = _1]
				>> *( ("==" >> inequation [_val == _1])
				    | ('=' >> inequation [_val == _1])
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

			variable = +qi::char_("xyvXYV");

			str_int =  (qi::char_("-") >> *qi::char_("0-9")) | (*qi::char_("0-9"));

			array_subscript = ("a[" >> str_int >> qi::char_(",") >> str_int >> ']') |
				("A[" >> str_int >> qi::char_(",") >> str_int >> ']');
			//array_subscript = ("a[" >> (*qi::char_("0-9")) >> (*qi::char_(",")) >> (*qi::char_("0-9")) >> ']')
			//		| ("A[" >> (*qi::char_("0-9")) >> (*qi::char_(",")) >> (*qi::char_("0-9")) >> ']');

			//("a[" >> (*qi::char_("0-9")) >> (*qi::char_(",")) >> (*qi::char_("0-9")) >> ']')


			function =
				( "min(" >> expression [_val = _1] >> ',' >> expression [_val = min_func(_val,_1)] >> ')')
				| ( "max(" >> expression [_val = _1] >> ',' >> expression [_val = max_func(_val,_1)] >> ')')
				| ( "abs(" >> expression [_val = abs_func(_1)] >> ')')
				| ( "sqrt(" >> expression [_val = sqrt_func(_1)] >> ')');

			factor =
				variable                        [_val = _1]
				| array_subscript                 [_val = _1]
				| uint_                         [_val = _1]
				|   '(' >> expression           [_val = _1] >> ')'
				|   ('-' >> factor              [_val = neg(_1)])
				|   ('+' >> factor              [_val = _1])
				|   ('!' >> factor              [_val = not_func(_1)])
				| function                  [_val = _1]
				;


		}
		qi::rule<Iterator, expression_ast(), ascii::space_type>
		expression, and_expression, equation, inequation, sum, term, factor, function;
		qi::rule<Iterator, std::string()> variable, array_subscript, str_int;
	};
}

/**
	Builds AST from equation which can be used later to solve it.
	Good if you have to compute the same equation multiple times.
	@param equation Equation string.
	@param ast Pointer to AST which will be assigned.
	@throw error strings on parse error
*/
void build_tree_from_equation(const char* equation, eqsolver::expression_ast* ast);

//! Computes the result of the equation with @a x, @a y and @a z filled in.
int solve_single_equation(const char* equation, int x, int y, int v);

#endif // EQUATION_SOLVER_H
