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

#ifndef EQUATION_SOLVER_H
#define EQUATION_SOLVER_H

#include <cmath>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
/*#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_char.hpp>*/
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
//#include <boost/phoenix/bind/bind_function.hpp>

#include "random.h"

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

// typedef expression_ast result_type; // TODO: must work with phoenix 3!

//! namespace of the equation solver
namespace eqsolver
{
inline int f1i_not(int arg1) { return (int)(arg1==0); }
inline int f1i_neg(int arg1) { return -arg1; }
inline int f1i_abs(int arg1) { return std::abs(arg1); }
inline int f1i_sqrt(int arg1) { return (int)std::sqrt((int)arg1); }
inline int f1i_rand(int arg1) { return (int)sca_random::get_int(arg1); }
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
inline int f2i_asn(int* arg1, int arg2) { return (*arg1 = arg2); }
inline int f2i_com(int arg1, int arg2) { (void)arg1; return arg2; }
inline int f3i_tern(int arg1, int arg2, int arg3) { return((bool)arg1)?arg2:arg3; }

// typedef int (*fptr_1i)(int arg1);
// typedef int (*fptr_2i)(int arg1, int arg2);

template<class Ret, class ...Args>
using fptr_base = Ret (*)(Args...);

// using fptr_1i = fptr_base<int, int>;
// using fptr_2i = fptr_base<int, int, int>;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template<class Ret, class ...Args>
struct ternary_op;
template<class Ret, class ...Args>
struct binary_op;
template<class Ret, class ...Args>
struct unary_op;

using ternary_op_i = ternary_op<int, int, int, int>;
using binary_op_i = binary_op<int, int, int>; // TODO: unused?
using unary_op_i = unary_op<int, int>;

struct visit_result_type
{
	boost::variant<unsigned int, int*> v;

	visit_result_type(unsigned int i) : v(i) {}
	visit_result_type(int* i) : v(i) {}

	// templates assure that the 1st version will be preferred
	//template<class T> visit_result_type(T& i) : v(i) {}
	//template<> visit_result_type(int* i) : v(i) {}

	visit_result_type() {}
	operator int() { return boost::get<unsigned int>(v); }
	operator int*() { return boost::get<int*>(v); }
};

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

	//! is used as "adress to index"
	template<bool IsAddress>
	struct var_helper {
		int i; //!< index that shall be adressed
		var_helper(std::string const& s/*, bool _address = false*/)
		 : i(atoi(s.c_str()))
		{
		}
	};
	typedef boost::variant<nil, var_x, var_y, var_array, var_helper<true>, var_helper<false>> type;
	type expr;
	vaddr(const type& _t) { expr = _t; }
	vaddr& operator=(const type& _expr) { expr = _expr; return *this; }
	vaddr() : expr(nil()) {}
};

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

const boost::phoenix::function< _make_2<vaddr::var_array> > make_array_indexes;
const boost::phoenix::function< _make_1<vaddr::var_helper<true>> > make_helper_index;
const boost::phoenix::function< _make_1<vaddr::var_helper<false>> > make_helper_index_var;
//const boost::phoenix::function< _make_1<vaddr::var_helper> > make_helper_index;
const boost::phoenix::function< _make_0<vaddr::var_x> > make_x;
const boost::phoenix::function< _make_0<vaddr::var_y> > make_y;

struct variable_print : public boost::static_visitor<visit_result_type>
{
	variable_print(int _x) : x(_x) {}
	/**
	 * @brief variable_print
	 * @param _height
	 * @param _width
	 * @param _x
	 * @param _y
	 * @param _v The address of the current element in the grid. Needed for neighbor calculations.
	 * @param _h
	 */
	variable_print(int _height, int _width, int _x, int _y, const int* _v, const int* _h)
		: height(_height), width(_width), x(_x), y(_y),
		v((const int*)_v), helper_vars((int*)_h) {
	}

//	typedef unsigned int result_type;

	int height, width; // TODO: height unused?
	unsigned int x,y;
	const int *v;
	int* helper_vars;

	inline unsigned int operator()(nil) const { return 0; }
	inline result_type operator()(vaddr::var_x _x) const { (void)_x; return x; }
	inline result_type operator()(vaddr::var_y _y) const { (void)_y; return y; }
	inline result_type operator()(vaddr::var_array _a) const { return v[_a.x+_a.y*width]; }
	inline int* operator()(vaddr::var_helper<true> _h) const {
		//return (_h.address) ? ((result_type*)(helper_vars + _h.i)) : helper_vars[_h.i];
		return helper_vars + _h.i;
	}
	inline unsigned int operator()(vaddr::var_helper<false> _h) const {
		//return (_h.address) ? ((result_type*)(helper_vars + _h.i)) : helper_vars[_h.i];
		return helper_vars[_h.i];
	}

};

struct variable_area_grid : public boost::static_visitor<visit_result_type>
{
	inline unsigned int operator()(nil) const { return 0; }
	inline unsigned int operator()(vaddr::var_x _x) const { return 0; }
	inline unsigned int operator()(vaddr::var_y _y) const { return 0; }
	inline result_type operator()(vaddr::var_array _a) const {
		return std::max(std::abs(_a.x), std::abs(_a.y));
	}
	template<bool T>
	inline unsigned int operator()(vaddr::var_helper<T> _h) const { (void)_h; return 0; }
};


struct variable_area_helpers : public boost::static_visitor<visit_result_type>
{
	inline int operator()(nil) const { return -1; }
	inline result_type operator()(vaddr::var_x _x) const { return -1; }
	inline result_type operator()(vaddr::var_y _y) const { return -1; }
	inline result_type operator()(vaddr::var_array _a) const { (void)_a; return -1; }

	inline result_type operator()(vaddr::var_helper<true> _h) const { return _h.i; } // todo: correct?
	inline result_type operator()(vaddr::var_helper<false> _h) const { return _h.i; } // todo: correct?
};

//! expression tree node
struct expression_ast
{
/*	using type = boost::variant<
		nil
		, unsigned int
		, std::string  // TODO: needed?
		, vaddr
		, boost::recursive_wrapper<expression_ast>
		, boost::recursive_wrapper<binary_op<Ret, Args...>>
		, boost::recursive_wrapper<unary_op<Ret, Args...>>
		>;*/
	using type = boost::variant<
		nil
		, unsigned int
		, std::string  // TODO: needed?
		, vaddr
		, boost::recursive_wrapper<expression_ast>
		, boost::recursive_wrapper<ternary_op<int, int, int, int>>
		, boost::recursive_wrapper<binary_op<int, int, int>>
		, boost::recursive_wrapper<binary_op<int, int*, int>>
		, boost::recursive_wrapper<unary_op<int, int>>
		>;

	expression_ast() : expr(nil()) {}

	template <typename T> struct factory_f {
		template<typename> struct result { typedef T type; };
		template<typename A> T operator()(A const& a) const { return T(a); }
	};

	expression_ast(expression_ast const& ai) : expr(ai.expr) {}
	explicit expression_ast(type const& ai) : expr(ai) {}

	template <typename Expr>
	explicit expression_ast(Expr const& expr) : expr(expr) {}

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


// TODO: we might want to introduce "nary_op"
//! expression tree node extension for ternary operators
template<class Ret, class ...Args>
struct ternary_op
{
	// below, (*) is a little fix to allow
	// expression_ast(type const& ai) to be explicit
	ternary_op(
		char op
		, fptr_base<Ret, Args...> fptr
		, expression_ast::type const& left // (*)
		, expression_ast const& middle
		, expression_ast const& right)
	: op(op), fptr(fptr), left(left), middle(middle), right(right) {}

	char op;
	fptr_base<Ret, Args...> fptr;
	expression_ast left, middle, right;
};

//! expression tree node extension for binary operators
template<class Ret, class ...Args>
struct binary_op
{
	// below, (*) is a little fix to allow
	// expression_ast(type const& ai) to be explicit
	binary_op(
		char op
		, fptr_base<Ret, Args...> fptr
		, expression_ast::type const& left // (*)
		, expression_ast const& right)
	: op(op), fptr(fptr), left(left), right(right) {}

	char op;
	fptr_base<Ret, Args...> fptr;
	expression_ast left;
	expression_ast right;
};

//! expression tree node extension for unary operators
template<class Ret, class ...Args>
struct unary_op
{
	unary_op(char op, fptr_base<Ret, Args...> fptr,
		expression_ast const& subject)
	: op(op), fptr(fptr), subject(subject) {}

	char op;
	fptr_base<Ret, Args...> fptr;
	expression_ast subject;
};

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
struct ast_print  : public boost::static_visitor<visit_result_type>
{


	const variable_handler* var_print;

	// h and w shall be internal, since we want to avoid adding 2
	// so it is still general to non-bordered areas
//	int height, width;
	typedef unsigned int res_type;
//	const res_type x,y, *v;
	//mutable std::map<int, int> helper_vars;
//	variable_print var_print;

//	inline int position(int _x, int _y) const { return (_y*(width+1)+_x+1); }

	inline res_type operator()(const eqsolver::nil&) const { return 0; }

	inline res_type operator()(qi::info::nil) const { return 0; }
	inline res_type operator()(int n) const { return n; }
	inline res_type operator()(std::string c) const
	{
		exit(99);
	}

	inline visit_result_type operator()(const vaddr& v) const
	{
		return boost::apply_visitor(*var_print, v.expr);
	}

	inline visit_result_type operator()(expression_ast const& ast) const
	{
		//ast_area helper_num(ast_area::MAX_HELPER);
		//helper_vars = new int[helper_num()+1];
		// TODO: delete int[]
		return boost::apply_visitor(*this, ast.expr);
	}

	template<class Ret, class ...Args>
	inline res_type operator()(ternary_op<Ret, Args...> const& expr) const
	{
		auto left = boost::apply_visitor(*this, expr.left.expr);
		auto middle = boost::apply_visitor(*this, expr.middle.expr);
		auto right = boost::apply_visitor(*this, expr.right.expr);

		return expr.fptr(left, middle, right);
	}

	template<class Ret, class ...Args>
	inline res_type operator()(binary_op<Ret, Args...> const& expr) const
	{
		auto left = boost::apply_visitor(*this, expr.left.expr);
		auto right = boost::apply_visitor(*this, expr.right.expr);
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

/*	inline res_type operator()(binary_op<int, int*, int> const& expr) const
	{
		int left = boost::apply_visitor(*this, expr.left.expr);
		int right = boost::apply_visitor(*this, expr.right.expr);
		return expr.fptr(left, right);
	}*/

	inline res_type operator()(unary_op<int, int> const& expr) const {
		return expr.fptr(boost::apply_visitor(*this, expr.subject.expr));
	}


	ast_print(const variable_handler* _var_print) : var_print(_var_print) {}

//	ast_print(int x, int y, int* v) : x(x), y(y), v((result_type*)v) {}

};

//! Class for iterating an expression tree and print the used area in the array.
//! The result is an int describing the half size of a square.
template<typename variable_handler>
struct ast_area  : public boost::static_visitor<unsigned int>
{
	typedef unsigned int result_type;
	variable_handler var_area;

	inline result_type operator()(const eqsolver::nil&) const { return 0; }

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

	template<class Ret, class ...Args>
	inline result_type operator()(binary_op<Ret, Args...> const& expr) const
	{
		return std::max( // TODO: fix everything into signed int
			(int) boost::apply_visitor(*this, expr.left.expr),
			(int) boost::apply_visitor(*this, expr.right.expr)
		);
	}

	template<class Ret, class ...Args>
	inline result_type operator()(ternary_op<Ret, Args...> const& expr) const
	{
		return
		std::max(
		std::max(
			(int) boost::apply_visitor(*this, expr.left.expr),
			(int) boost::apply_visitor(*this, expr.right.expr)
		), (int) boost::apply_visitor(*this, expr.middle.expr));
	}

	template<class Ret, class ...Args>
	inline result_type operator()(unary_op<Ret, Args...> const& expr) const {
		return boost::apply_visitor(*this, expr.subject.expr);
	}

	//ast_area(variable_area::AREA_TYPE _area_type)
	//	: var_area(_area_type) {}
};

const char* get_help_description();

/**
	Builds AST from equation which can be used later to solve it.
	Good if you have to compute the same equation multiple times.
	@param equation Equation string.
	@param ast Pointer to AST which will be assigned.
	@throw error strings on parse error
*/
void build_tree(const char* equation, eqsolver::expression_ast* ast);

//! Computes the result of the equation with @a x, @a y and @a z filled in.
int solve(const char* equation, int x, int y, int v);

}

#endif // EQUATION_SOLVER_H
