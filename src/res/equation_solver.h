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

#include <map>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
//#include <boost/phoenix/bind/bind_function.hpp>

const char* get_help_description();

/*
	Again, I think these macros are okay here.
*/
#define MAKE_UNARY_FUNC(OP, VISUAL, EXECUTE) struct OP \
{ \
	template <typename T> \
	struct result { typedef T type; }; \
	inline expression_ast operator()(expression_ast const& expr) const { \
		return expression_ast(unary_op(VISUAL, EXECUTE, expr)); \
	} \
}; \
const boost::phoenix::function<OP> OP;

#define MAKE_BINARY_FUNC(OP, VISUAL, EXECUTE) struct OP \
{ \
	template <typename T1, typename T2> \
	struct result { typedef T2 type; }; \
	inline expression_ast operator()(expression_ast const& expr1, expression_ast const& expr2) const { \
		return expression_ast(binary_op(VISUAL, EXECUTE, expr1, expr2)); \
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
	inline int f2i_com(int arg1, int arg2) { (void)arg1; return arg2; }

	typedef int (*fptr_1i)(int arg1);
	typedef int (*fptr_2i)(int arg1, int arg2);

	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	struct binary_op;
	struct unary_op;
	//! empty expression, can't happen!
	struct nil {};

	struct vaddr
	{
		struct var_x{};
		struct var_y{};
		struct var_array {
			int x; int y;
			var_array(std::string const& s1, std::string const& s2) {
				x = atoi(s1.c_str()); y = atoi(s2.c_str());
			}
		};
		struct var_helper {
			int i;
			var_helper(std::string const& s) { i = atoi(s.c_str()); }
		};
		typedef boost::variant<nil, var_x, var_y, var_array, var_helper> type;
		type expr;
		vaddr(const type& _t) { expr = _t; }
		vaddr& operator=(const type& _expr) { expr = _expr; return *this; }
		vaddr() : expr(nil()) {}
	};

/*	struct make_array_indexes
	{
		template <typename T1, typename T2>
		struct result { typedef vaddr type; };
		// TODO: allow expressions for array indixes??
		template <typename T1, typename T2>
		inline typename result<T1, T2>::type operator()(T1 const& c1, T2 const& c2) const {
			return vaddr(
				vaddr::var_array(c1, c2));
		}
	};
	const boost::phoenix::function<make_array_indexes> make_array_indexes;*/


	/*struct make_helper_index
	{
		template <typename T, bool>
		struct result { typedef vaddr type; }; // TODO: ref string?

		template <typename T, bool>
		inline typename result<T, bool>::type operator()(T const& c1, bool pointer) const {
			return vaddr(
				vaddr::var_helper(c1, pointer));
		}
	};
	const boost::phoenix::function<make_helper_index> make_helper_index;*/



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
		template <typename T>
		struct result { typedef _result_type type; };

		template <typename T>
		inline typename result<T>::type operator()(T const& c1) const {
			return _result_type(c1);
		}
	};
	template<typename _result_type>
	struct _make_2
	{
		template <typename T1, typename T2>
		struct result { typedef _result_type type; };

		template <typename T1, typename T2>
		inline typename result<T1, T2>::type operator()(
				T1 const& c1, T2 const& c2) const {
			return _result_type(c1, c2);
		}
	};

	const boost::phoenix::function< _make_2<vaddr::var_array> > make_array_indexes;
	const boost::phoenix::function< _make_1<vaddr::var_helper> > make_helper_index;
	const boost::phoenix::function< _make_0<vaddr::var_x> > make_x;
	const boost::phoenix::function< _make_0<vaddr::var_y> > make_y;

	inline vaddr::var_x make_x_bound_function(void) { return vaddr::var_x(); }

	/*struct _make_x
	{
		typedef vaddr result_type;
		inline result_type operator()() const {
			return vaddr( vaddr::var_x() );
		}
	};
	const boost::phoenix::function<_make_x> make_x;

	struct _make_y
	{
		typedef vaddr result_type;
		inline result_type operator()() const {
			return vaddr( vaddr::var_y() );
		}
	};
	const boost::phoenix::function<_make_y> make_y;*/

	struct variable_print : public boost::static_visitor<>
	{
		variable_print(int _x) : x(_x) {}
		variable_print(int _height, int _width, int _x, int _y, const int* _v, const int* _h)
			: height(_height), width(_width), x(_x), y(_y),
			v((const result_type*)_v), helper_vars((result_type*)_h) {
		}

		typedef unsigned int result_type;
		int height, width; // TODO: height unused?
		result_type x,y;
		const result_type *v;
		result_type* helper_vars;

		inline result_type operator()(nil) const { return 0; }
		inline result_type operator()(vaddr::var_x _x) const { (void)_x; return x; }
		inline result_type operator()(vaddr::var_y _y) const { (void)_y; return y; }
		inline result_type operator()(vaddr::var_array _a) const { return v[_a.x+_a.y*width]; }
		inline result_type operator()(vaddr::var_helper _h) const { return helper_vars[_h.i]; }
	};

	struct variable_area_grid : public boost::static_visitor<>
	{
		typedef unsigned int result_type;
		inline result_type operator()(nil) const { return 0; }
		inline result_type operator()(vaddr::var_x _x) const { return 0; }
		inline result_type operator()(vaddr::var_y _y) const { return 0; }
		inline result_type operator()(vaddr::var_array _a) const {
			return std::max(std::abs(_a.x), std::abs(_a.y));
		}
		inline result_type operator()(vaddr::var_helper _h) const { (void)_h; return 0; }
	};


	struct variable_area_helpers : public boost::static_visitor<>
	{
		typedef unsigned int result_type;
		inline result_type operator()(nil) const { return -1; }
		inline result_type operator()(vaddr::var_x _x) const { return -1; }
		inline result_type operator()(vaddr::var_y _y) const { return -1; }
		inline result_type operator()(vaddr::var_array _a) const { (void)_a; return -1; }
		inline result_type operator()(vaddr::var_helper _h) const { return _h.i; }
	};

	//! expression tree node
	struct expression_ast
	{
		typedef boost::variant<
			nil
			, unsigned int
			, std::string  // TODO: needed?
			, vaddr
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
	// in phoenix type deduction mechanism that prevents us fom
	// doing so. Phoenix will be switching to BOOST_TYPEOF. In the
	// meantime, we will use a phoenix::function below:
	MAKE_UNARY_FUNC(neg, '-', f1i_neg);
	MAKE_UNARY_FUNC(not_func, '!', f1i_not);
	MAKE_UNARY_FUNC(abs_func, 'a', f1i_abs);
	MAKE_UNARY_FUNC(sqrt_func, 's', f1i_sqrt);
	MAKE_BINARY_FUNC(min_func, 'm', f2i_min);
	MAKE_BINARY_FUNC(max_func, 'n', f2i_max);
	MAKE_BINARY_FUNC(ass_func, 'h', NULL);
	MAKE_BINARY_FUNC(com_func, ',', f2i_com);

	inline int array_subscript_to_coord(std::string* str, int width) {
		const int comma_pos = str->find(',');
		const int first_pos = str->find('[')+1;
		(*str)[comma_pos]=0;
		(*str)[str->find(']')]=0;
		//printf("coords: %s, %s\n",str->data()+first_pos, str->data()+(comma_pos+1));
		return atoi(str->data()+first_pos)+atoi(str->data()+(comma_pos+1))*width;
	}

	//! Class for iterating an expression tree and printing the result.
	template<typename variable_handler>
	struct ast_print
	{
		const variable_handler* var_print;

		// h and w shall be internal, since we want to avoid adding 2
		// so it is still general to non-bordered areas
	//	int height, width;
		typedef unsigned int result_type;
	//	const result_type x,y, *v;
		//mutable std::map<int, int> helper_vars;
	//	variable_print var_print;

	//	inline int position(int _x, int _y) const { return (_y*(width+1)+_x+1); }

		inline result_type operator()(qi::info::nil) const { return 0; }
		inline result_type operator()(int n) const { return n;  }
		inline result_type operator()(std::string c) const
		{
			exit(99);
		}

		inline result_type operator()(const vaddr& v) const
		{
			return boost::apply_visitor(*var_print, v.expr);
		}

		inline result_type operator()(expression_ast const& ast) const
		{
			//ast_area helper_num(ast_area::MAX_HELPER);
			//helper_vars = new int[helper_num()+1];
			// TODO: delete int[]
			return boost::apply_visitor(*this, ast.expr);
		}

		inline result_type operator()(binary_op const& expr) const
		{
			int left = boost::apply_visitor(*this, expr.left.expr);
			int right = boost::apply_visitor(*this, expr.right.expr);
			/*if(expr.fptr == NULL) { // this is an assignment
				puts("before");
				printf("before2:%s\n",boost::get<std::string>(expr.right.expr).c_str());
				left = atoi(
					boost::get<std::string>(expr.right.expr).data()+1);
				printf("after\n");
				helper_vars[left]=right;
				return right;
			}
			else*/
			return expr.fptr(left, right);
		}

		inline result_type operator()(unary_op const& expr) const {
			return expr.fptr(boost::apply_visitor(*this, expr.subject.expr));
		}

		ast_print(const variable_handler* _var_print) : var_print(_var_print) {}

	//	ast_print(int x, int y, int* v) : x(x), y(y), v((result_type*)v) {}

	};

	//! Class for iterating an expression tree and print the used area in the array.
	//! The result is an int describing the half size of a square.
	template<typename variable_handler>
	struct ast_area
	{
		typedef unsigned int result_type;
		variable_handler var_area;

		inline result_type operator()(qi::info::nil) const { return 0; }
		inline result_type operator()(int n) const { return 0;  }
		inline result_type operator()(std::string c) const
		{
			exit(99);
		}

		inline result_type operator()(const vaddr& v) const
		{
			return boost::apply_visitor(var_area, v.expr);
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

		//ast_area(variable_area::AREA_TYPE _area_type)
		//	: var_area(_area_type) {}
	};

	//! caluclator grammar to build expression trees
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

			str_int =  (qi::char_("-") >> *qi::char_("0-9")) | (*qi::char_("0-9"));

			helper_variable = "h[" >> (str_int [_val = make_helper_index(_1)]) >> ']';
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




			variable = qi::char_("x") [ /*_val = make_x()*/ _val = boost::phoenix::bind(&make_x_bound_function)() ]
				| qi::char_("y") [ _val = make_y() ]
				| qi::char_("v") [ _val = make_array_indexes(null_str, null_str)];

			//helper_variable = qi::char_("h") >> qi::char_("[") >> str_int >> qi::char_("]");
			//array_variable = qi::char_("a") >> qi::char_("[") >> str_int >> qi::char_(",") >> str_int >> qi::char_("]");




			comma_assignment = assignment [_val = _1]
					>> *( (',' >> assignment [_val = com_func(_val, _1)]));
			assignment = (helper_variable [_val = _1] >> '='
				>> expression [_val = ass_func(_val,_1)]) | expression [_val = _1];

			expression = and_expression [_val = _1]
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
				variable                        [_val = _1]
				//| array_subscript               [_val = _1]
				| helper_variable [_val = _1]
				| array_variable  [_val = _1]
				| uint_                         [_val = _1]
				|   '(' >> comma_assignment           [_val = _1] >> ')'
				|   ('-' >> factor              [_val = neg(_1)])
				|   ('+' >> factor              [_val = _1])
				|   ('!' >> factor              [_val = not_func(_1)])
				| function                  [_val = _1]
				;


		}
		qi::rule<Iterator, expression_ast(), ascii::space_type> comma_assignment, assignment,
		expression, and_expression, equation, inequation, sum, term, factor, function;
		qi::rule<Iterator, vaddr()> variable, array_variable, helper_variable;
		qi::rule<Iterator, std::string()> str_int;
		//qi::rule<Iterator, std::string()> array_variable, helper_variable, variable, array_subscript, str_int;
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
