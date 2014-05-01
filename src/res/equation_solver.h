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

#include <boost/spirit/include/support_info.hpp> // qi::info::nil

//#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
//#include <boost/phoenix/bind/bind_function.hpp>

#include "random.h"

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

//namespace qi = boost::spirit::qi;
//namespace ascii = boost::spirit::ascii;

template<std::size_t N, class Ret, class ...Args>
struct nary_op;

template<class Ret, class ...Args>
using ternary_op = nary_op<3, Ret, Args...>;
template<class Ret, class ...Args>
using binary_op = nary_op<2, Ret, Args...>;
template<class Ret, class ...Args>
using unary_op = nary_op<1, Ret, Args...>;

template<std::size_t... Is> struct seq {};
template<std::size_t N, std::size_t... Is> struct gen_seq : gen_seq<N-1, N-1, Is...> {};
template<std::size_t... Is> struct gen_seq<0, Is...> : seq<Is...> {};

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
	inline unsigned int operator()(vaddr::var_x) const { return 0; }
	inline unsigned int operator()(vaddr::var_y) const { return 0; }
	inline result_type operator()(vaddr::var_array _a) const {
		return std::max(std::abs(_a.x), std::abs(_a.y));
	}
	template<bool T>
	inline unsigned int operator()(vaddr::var_helper<T> _h) const { (void)_h; return 0; }
};


struct variable_area_helpers : public boost::static_visitor<visit_result_type>
{
	inline int operator()(nil) const { return -1; }
	inline result_type operator()(vaddr::var_x) const { return -1; }
	inline result_type operator()(vaddr::var_y) const { return -1; }
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

//! expression tree node extension for ternary operators
template<std::size_t N, class Ret, class ...Args>
struct nary_op
{
	// below, (*) is a little fix to Sallow
	// expression_ast(type const& ai) to be explicit
	template<class ...AstClass>
	nary_op(
		char op
		, fptr_base<Ret, Args...> fptr
		, expression_ast::type const& left // (*)
		, AstClass const& ... more)
	: op(op), fptr(fptr), subtrees({(expression_ast)left, more...}) {}

	char op;
	fptr_base<Ret, Args...> fptr;
	expression_ast subtrees[N];
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

	inline res_type operator()(boost::spirit::info::nil) const { return 0; }
	inline res_type operator()(int n) const { return n; }
	inline res_type operator()(std::string) const
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
private:
	template<class NaryOpT, std::size_t ...Idxs>
	inline res_type apply_fptr(NaryOpT const& expr, seq<Idxs...>) const
	{
		return expr.fptr(
			boost::apply_visitor(*this, expr.subtrees[Idxs].expr)...
		);
	}

public:
	template<std::size_t N, class Ret, class ...Args>
	inline res_type operator()(nary_op<N, Ret, Args...> const& expr) const
	{
		return apply_fptr(expr, gen_seq<N>());
	}

	ast_print(const variable_handler* _var_print) : var_print(_var_print) {}

//	ast_print(int x, int y, int* v) : x(x), y(y), v((result_type*)v) {}

};

//! computes int max of n values
template<class ...IntT>
inline int n_max(int first, IntT ...more) {
	return std::max(first, n_max(more...));
}

template<>
inline int n_max(int first) {
	return first;
}

//! Class for iterating an expression tree and print the used area in the array.
//! The result is an int describing the half size of a square.
template<typename variable_handler>
struct ast_area  : public boost::static_visitor<unsigned int>
{
	typedef unsigned int result_type;
	variable_handler var_area;

	inline result_type operator()(const eqsolver::nil&) const { return 0; }

	inline result_type operator()(boost::spirit::info::nil) const { return 0; }
	inline result_type operator()(int) const { return 0;  }
	inline result_type operator()(std::string) const
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

private:
	template<class NaryOpT, std::size_t ...Idxs>
	inline result_type apply_fptr(NaryOpT const& expr, seq<Idxs...>) const
	{
		return n_max(
			(int)boost::apply_visitor(*this, expr.subtrees[Idxs].expr)...
			// TODO: this cast is dangerous!
			// (the result is sometimes a pointer, somehow)
		);
	}
public:
	template<std::size_t N, class Ret, class ...Args>
	inline result_type operator()(nary_op<N, Ret, Args...> const& expr) const {
		return apply_fptr(expr, gen_seq<N>());
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
