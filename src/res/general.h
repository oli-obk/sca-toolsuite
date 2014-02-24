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

#include <cassert>
#include <exception>
#include <climits>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>
#include <string>

#include "random.h"

#ifndef GENERAL_H
#define GENERAL_H
#define SCA_DEBUG

//! Generic structure for a 2D rectangle dimension.
struct dimension
{
	unsigned int height;
	unsigned int width;
	inline unsigned int area() const { return height*width; }
	inline unsigned int area_without_border() const {
		assert(height > 1); assert(width > 1);
		return (height-2)*(width-2);
	}
	inline bool operator==(const dimension& other) {
		return height == other.height && width == other.width;
	}
	inline bool operator!=(const dimension& other) { return !(operator ==(other)); }
};

//! Returns true iff @a idx is on the border for given dimension @a dim
inline bool is_border(const dimension* dim, unsigned int idx) {
	const unsigned int idx_mod_width = idx % dim->width;
	return (idx<dim->width || idx>= dim->area()-dim->width
	|| idx_mod_width == 0 || idx_mod_width==dim->width-1);
}

//! Given an empty vector @a grid, creates grid of dimension @a dim
//! with all cells being @a predefined_value
//! @todo This is highly inefficient. Now in O(n^2), can be done easily in O(n)
inline void create_empty_grid(std::vector<int>* grid, const dimension* dim,
	int predefined_value = 0)
{
	grid->resize(dim->area(), predefined_value);
	for(unsigned int i=0; i<dim->area(); i++)
	{
		if(is_border(dim, i))
		 (*grid)[i] = INT_MIN;
	}
}

inline bool human_idx_on_grid(const int human_grid_size, const int human_idx) {
	return (human_idx >= 0 && human_idx < human_grid_size);
}

//! Generic structure to store help commandline information in and print it
struct HelpStruct
{
	const char* description;
	const char* syntax;
	const char* input;
	const char* output;
	typedef std::vector<std::pair<const char*, const char*> > param_vec;
	param_vec params;
	const char* return_value;
	inline void print_usage () const {
		if(syntax) fprintf(stderr, "Usage: %s\n", syntax);
	}
	void print_help() const
	{
		fprintf(stderr, "%s\n", description);

		if(input||output) {
			fputs("\n",stderr);
			if(input) fprintf(stderr, "Input (stdin): %s\n", input);
			if(output) fprintf(stderr, "Output (stdout): %s\n", output);
		}

		fputs("\n",stderr);
		print_usage();
		if(params.size()) fprintf(stderr, "Arguments:\n");
		for(param_vec::const_iterator itr = params.begin();
			itr != params.end(); itr++)
		 fprintf(stderr, " * %s: %s\n", itr->first, itr->second);

		if(return_value) fprintf(stderr, "Return value: %s\n", return_value);
		fputs("\nSyntax rules are explained in the file DOCUMENTATION.\n", stderr);
	}
	inline void add_param(const char* name, const char* descr) {
		params.push_back(std::make_pair(name,descr));
	}

	HelpStruct()
		: description(NULL), syntax(NULL), // non-NULL params
		input(NULL), output(NULL), return_value(NULL) {}
};

//! Helper struct for all programs for help displaying and error handling.
class Program
{
protected:
	const bool env_debug;
	int argc;
	char** argv;
private:
	const HelpStruct* help;

	inline void print_termination_string(void) const {
		fprintf(stderr, "%s: Terminating on fatal error.\n", argv[0]);
	}
	inline void internal_exit(void) const { ::exit(EXIT_FAILURE); }

public:
	//! Routine for inherited classes to call to start main().
	int run(int _argc, char** _argv, const HelpStruct* _help)
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
		}/* catch(std::exception e) {
			exit(e.what());
		} catch(...) {
			exit("Unknown error caught. This should never happen.");
		}*/
		return return_value;
	}
	static inline int safe_atoi(const char* str) { return str?atoi(str):0; }
	Program() : env_debug(safe_atoi(getenv("SCA_DEBUG"))) {}
	virtual ~Program() {}

protected:
	//! Main routine for inherited classes.
	virtual int main() = 0;

	//! Exit program with message @a str
	inline void exit(const char* str) const
	{
		print_termination_string();
		fprintf(stderr, "%s\n", str);
		internal_exit();
	}

	//! Exit program with format string @a format
	inline void exitf(const char* format, ...) const
	{
		print_termination_string();
		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		internal_exit();
	}

	//! Prints str if shell sets SCA_DEBUG and macro SCA_DEBUG is defined.
	//! Programmers can put these everywhere without runtime worries.
	inline void debug(const char* str) const {
#ifdef SCA_DEBUG
		if(env_debug) fputs(str, stderr);
#endif
	}

	//! Prints format if shell sets SCA_DEBUG and macro SCA_DEBUG is
	//! defined.
	//! Programmers can put these everywhere without runtime worries
	inline void debugf(const char* format, ...) const
	{
#ifdef SCA_DEBUG
		if(env_debug)
		{
			va_list args;
			va_start(args, format);
			vfprintf(stderr, format, args);
			va_end(args);
		}
#endif
	}

	//! Exit program with usage
	inline void exit_usage() const
	{
		help->print_usage();
		fprintf(stderr, " (--help for more information)\n");
		internal_exit();
	}

	//! Exit program if @a assertion is false, printing usage
	inline void assert_usage(bool assertion) const { if(!assertion) exit_usage(); }
};

#endif // GENERAL_H
