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

#include <cstdlib> // atoi(), exit()
#include <cassert>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include <exception>
#include "general.h"

void os_sleep(unsigned int seconds) { sleep(seconds); }
void os_clear() { system("clear"); }

void Program::print_termination_string() const {
	std::cerr << argv[0] << ": Terminating on fatal error" << std::endl;
}

void Program::internal_exit() const { ::exit(EXIT_FAILURE); }

int Program::safe_atoi(const char *str) { return str ? atoi(str) : 0; }

int Program::run(int _argc, char **_argv, const HelpStruct *_help)
{
	argc = _argc; argv = _argv; help = _help;

	assert(help->description && help->syntax);
	if(argc > 1 && (!strcmp(argv[1],"--help")||!strcmp(argv[1],"-help")))
		{
			help->print_help();
			::exit(0);
		}

	int return_value = 0;
	try {
		return_value = main();
	} catch(const char* str) {
		exit(str);
	} catch(const std::string& str) {
		exit(str.c_str());
	} catch(std::exception e) {
		exit(e.what());
	} catch(...) {
		exit("Unknown error caught. This should never happen.");
	}
	return return_value;
}

Program::Program() : env_debug(safe_atoi(getenv("SCA_DEBUG"))) {}

Program::~Program() {}

void Program::exit(const char *str) const
{
	print_termination_string();
	std::cerr << str << std::endl;
	internal_exit();
}

void Program::debug(const char *str) const {
#ifdef SCA_DEBUG
	if(env_debug) fputs(str, stderr);
#endif
}

void Program::exit_usage() const
{
	help->print_usage();
	fprintf(stderr, " (--help for more information)\n");
	internal_exit();
}

void Program::assert_usage(bool assertion) const { if(!assertion) exit_usage(); }


void HelpStruct::print_usage() const {
	if(syntax)
	 std::cerr << "Usage: " << syntax << std::endl;
}

void HelpStruct::print_help() const
{
	std::cerr << description << std::endl;

	if(input||output) {
		std::cerr << std::endl;
		if(input) std::cerr << "Input (stdin): " << input << std::endl;
		if(output) std::cerr << "Output (stdout): " << output << std::endl;
	}

	std::cerr << std::endl;
	print_usage();
	if(params.size())
		std::cerr << "Arguments:" << syntax << std::endl;
	for( const auto& p : params )
		std::cerr << " * " << p.first << ": " << p.second << std::endl;

	if(return_value) std::cerr << "Return value: " << return_value << std::endl;
	std::cerr << std::endl <<
		"Syntax rules are explained"
		"in the file DOCUMENTATION." << std::endl;
}

void HelpStruct::add_param(const char *name, const char *descr) {
	params.push_back(std::make_pair(name,descr));
}

HelpStruct::HelpStruct()
	: description(NULL), syntax(NULL), // non-NULL params
	  input(NULL), output(NULL), return_value(NULL) {}


void assert_always(bool assertion, const char *msg)
{
	if(!assertion)
	{
		std::cerr << "Error: " << msg << std::endl;
		::exit(EXIT_FAILURE);
	}
}
