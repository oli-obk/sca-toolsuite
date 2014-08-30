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

#ifndef GENERAL_H
#define GENERAL_H

#include <cstdio> // TODO: remove soon ?
#include <utility> // pair
#include <vector>

#define SCA_DEBUG

// TODO: correct here?
void os_sleep(unsigned int seconds);
void os_clear();

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
	void print_usage () const;
	void print_help() const;
	void add_param(const char* name, const char* descr);

	HelpStruct();
};

//! Exit program if @a assertion is false, printing msg
void assert_always(bool assertion, const char* msg);

//! Helper struct for all programs for help displaying and error handling.
class Program
{
protected:
	const bool env_debug;
	int argc;
	char** argv;

	enum class exit_t {
		success = 0,
		failure = 1
	};

	static int _exit_code(exit_t ec) { return (int)(ec); }
	static exit_t success(bool cond) {
		return cond ? exit_t::success : exit_t::failure;
	}
private:
	const HelpStruct* help;

	void print_termination_string(void) const;
	void internal_exit(void) const;
	static int safe_atoi(const char* str);

public:
	//! Routine for inherited classes to call to start main().
	int run(int _argc, char** _argv, const HelpStruct* _help);
	Program();
	virtual ~Program();

protected:
	//! Main routine for inherited classes.
	virtual exit_t main() = 0;

	//! Exit program with message @a str
	void exit(const char* str) const;

	//! Exit program with format string @a format
	template<class ...Args>
	inline void exitf(const char* format, Args... args) const
	{
		print_termination_string();
		fprintf(stderr, format, args...);
		internal_exit();
	}

	//! Prints str if shell sets SCA_DEBUG and macro SCA_DEBUG is defined.
	//! Programmers can put these everywhere without runtime worries.
	void debug(const char* str) const;

	//! Prints format if shell sets SCA_DEBUG and macro SCA_DEBUG is
	//! defined.
	//! Programmers can put these everywhere without runtime worries
	template<class ...Args>
	inline void debugf(const char* format, Args... args) const
	{
#ifdef SCA_DEBUG
		if(env_debug) fprintf(stderr, format, args...);
#endif
	}

	//! Exit program with usage
	exit_t exit_usage() const;

	//! Exit program if @a assertion is false, printing usage
	void assert_usage(bool assertion) const;
};

#endif // GENERAL_H
