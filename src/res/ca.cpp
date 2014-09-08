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

#include "ca.h"

namespace sca { namespace ca {

constexpr const uint32_t _ca_table_hdr_t::version_t::id;

ca_eqsolver_t::ca_eqsolver_t(const char *equation, unsigned num_states)
	: num_states(num_states)
{
	//	debug("Building AST from equation...\n");
	eqsolver::build_tree(equation, &ast);

	eqsolver::ast_area<eqsolver::variable_area_grid>
			grid_solver;
	_border_width = (int)grid_solver(ast);

	{
	eqsolver::ast_area_cont<eqsolver::variable_area_cont<std::set<point>>>
			grid_solver_2(true);
	std::set<point> res = grid_solver_2(ast);
	std::vector<point> res_v;
	std::move(res.begin(), res.end(), std::back_inserter(res_v));
	_n_in = n_t(std::move(res_v));
	center_cell = _n_in.get_center_cell();
	} // TODO: own function

	{
	eqsolver::ast_area_cont<eqsolver::variable_area_cont<std::set<point>>>
			grid_solver_2(false);
	std::set<point> res = grid_solver_2(ast);
	std::vector<point> res_v;
	std::move(res.begin(), res.end(), std::back_inserter(res_v));
	_n_out = n_t(std::move(res_v));
	}

#ifdef CA_DEBUG
	std::cout << "Input neighbourhood: " << _n_in << std::endl;
	std::cout << "Output neighbourhood: " << _n_out << std::endl;
#endif

#ifdef CA_DEBUG
	printf("Size of Moore Neighbourhood: %d\n", // TODO: use cout
	       _border_width);
#endif
	eqsolver::ast_area<eqsolver::variable_area_helpers>
			helpers_solver;
	helpers_size = (int)helpers_solver(ast) + 1;
#ifdef CA_DEBUG
	printf("Size of Helper Variable Array: %d\n",
	       helpers_size);
#endif
	if(helpers_size > 0)
		helper_vars = new int[helpers_size];

#if 0
	eqsolver::ast_minmax minmax_solver(helpers_size);
	//std::pair<int, int> mm = (std::pair<int, int>)minmax_solver(ast);
	/*	num_states = (mm.first == INT_MIN || mm.second == INT_MAX)
	? INT_MAX
	: (mm.second - mm.first + 1);*/
	std::pair<eqsolver::expression_ast, eqsolver::expression_ast> mm
			= (std::pair<eqsolver::expression_ast, eqsolver::expression_ast>)minmax_solver(ast);

	eqsolver::ast_dump dumper;
	std::cout << "original: " << dumper(ast) << std::endl;

	std::cout << "mm first: " << (std::string)dumper(mm.first) << std::endl; // TODO: non return syntax
	(void)mm;
#endif
	//num_states = 0;
}

_ca_table_hdr_t::size_check::size_check(int size)
{
	if(size < 0)
	 throw "Error: Size negative (did you set num_states == 0?).";
	if(size > (1<<18))
	 throw "Error: This ca is too large for a table.";
}

_ca_table_hdr_t::header_t::header_t(std::istream &stream)
{
	char buf[9];
	buf[8] = 0;
	stream.read(buf, 8);
	if(strcmp(buf, "ca_table"))
	 throw "Error: This file has no ca_table header.";
}

_ca_table_hdr_t::version_t::version_t(const uint32_t &i)
{
	if(i != id)
	{
		//char err_str[] = "Incompatible versions:\n" // 23
		//	"file: ................,\n" // 6 + 16 + 2
		//	"expected: ................";
		//sprintf(err_str + 23 + 6, "%016d", i);
		//sprintf(err_str + 23 + 6 + 16 + 2 + 10, "%016d", id);
		throw std::string("Incompatible versions: "
				  "read: " + std::to_string(i) +
				  ", expected: " + std::to_string(id) + ".");
	}
}

void _ca_table_hdr_t::dump(std::ostream &stream) const
{
	header_t::dump(stream);
	version_t::dump(stream);
	uint32_t tmp = n_w;
	stream.write((char*)&tmp, 4);
	tmp = own_num_states;
	stream.write((char*)&tmp, 4);
}

_ca_table_hdr_t::_ca_table_hdr_t(const char *equation, _ca_table_hdr_t::cell_t num_states) :
	base(equation, num_states),
	n_w((base::border_width()<<1) + 1),
	own_num_states(base::num_states),
	size_each((unsigned)ceil(log(own_num_states))), // TODO: use int arithm
	center(base::border_width(), base::border_width()),
	bw(compute_bw()),
	neighbourhood(compute_neighbourhood())
{
}

_ca_table_hdr_t::_ca_table_hdr_t(std::istream &stream) :
	base("v", 0), // not reliable
	header(stream),
	version(fetch_32(stream)),
	n_w(fetch_32(stream)),
	own_num_states(fetch_32(stream)),
	size_each((unsigned)ceil(log(own_num_states))), // TODO: use int arithm
	center((n_w - 1)>>1, (n_w - 1)>>1),
	bw(compute_bw()),
	neighbourhood(compute_neighbourhood())
{
}

} }
