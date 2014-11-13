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

#include <memory>
#include <sys/resource.h>
#include <csignal>

#include "general.h"

//#include "split/split.h"
//#include "impl/split_try.h"
#include "greedy.h"

void signal_handler(int signal_id)
{
	printf("Caught signal %d.\n",signal_id);
	if(base::global_abort)
	 throw "Aborting due to signal being received.";
	else
	{
		puts("Aborting softly. Hit ctrl+c again to abort instantly.\n");
		base::global_abort = true;
	}
}

void catch_sigint()
{
	struct sigaction handler;

	handler.sa_handler = signal_handler;
	sigemptyset(&handler.sa_mask);
	handler.sa_flags = 0;

	sigaction(SIGINT, &handler, NULL);
}

// TODO: http://stackoverflow.com/questions/2275550/
//	change-stack-size-for-a-c-application-
//	in-linux-during-compilation-with-gnu-com
void increase_stack_size(std::size_t megabyte)
{
	const rlim_t kStackSize = megabyte * 1024 * 1024;   // min stack size = 16 MB
	struct rlimit rl;
	int result;

	result = getrlimit(RLIMIT_STACK, &rl);
	if (result == 0)
	if (rl.rlim_cur < kStackSize)
	{
		rl.rlim_cur = kStackSize;
		result = setrlimit(RLIMIT_STACK, &rl);
		if (result != 0)
		{
			fprintf(stderr, "setrlimit returned result = %d\n", result);
		}
	}
}

// TODO: StackOverflow
std::string get_file_contents(const char *filename) // TODO: move?
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	else
	 throw("Could not open the specified file");
}

class MyProgram : public Program
{
	exit_t main()
	{
		exit_t result = exit_t::success;

		const char* tbl_file = nullptr;
		const char* type = "split";
		bool dump_on_exit = true;
		int dead_state = std::numeric_limits<int>::max();
		bool pipe = false;

		catch_sigint();

		/*
		 * args
		 */

		assert_usage(argc >= 3);
		switch(argc)
		{
			case 6:
				assert_usage(!strcmp(argv[5], "pipe"));
				pipe = true;
			case 5:
				type = argv[4];
			case 4:
				assert_usage(!strcmp(argv[3], "nodump")
					|| !strcmp(argv[3], "dump"));
				dump_on_exit = (!strcmp(argv[3], "dump"));
			case 3:
				assert_usage((argv[2][1] == 0) && isdigit(argv[2][0]));
				dead_state = atoi(argv[2]);
			case 2:
				tbl_file = argv[1];
				break;
			default:
				exit_usage();
		}

		/*
		 * parsing
		 */

		std::ifstream in(tbl_file, std::ios::in);

		std::ofstream outfile;
		if(!pipe)
		 outfile.open("results.dat");
		std::ostream& out = pipe ? std::cout : outfile;

		// TODO: error if in is not existing

		{

		std::unique_ptr<base> algo = nullptr;
		if(!strcmp(type, "greedy"))
		{
			algo.reset(new greedy::algo(in, dead_state, dump_on_exit));
		}
		else
		 throw "Unknown algorithm type specified";

		algo->parse();

		/*
		 * algorithm
		 */

		try {
			algo->run(out);
		}
		catch(const char* s)
		{
			if(dump_on_exit)
			{
				std::cout << "Aborting algorithm, reason: " << s << std::endl;
				std::cout << "Writing debug_graph.dot ..." << std::endl;
				std::ofstream final_graph("debug_graph.dot");
				final_graph << (base&)*algo;
			}
			else
			{
				std::cout << "Aborting algorithm, reason: " << s << std::endl;
				std::cout << "Dumping disabled by parameters" << std::endl;
			}
			result = exit_t::failure;
		}

		}

		//std::cout << "FINAL RESULTS: " << std::endl << *algo << std::endl;

		if(result == exit_t::success)
			std::cerr << std::endl
				<< "Success. Now call: `./eval help < results.dat'."
				<< std::endl << std::endl;
		else
			std::cerr << std::endl
				<< "Failure."
				<< std::endl << std::endl;

		return result;
	}
};

int main(int argc, char** argv)
{
	increase_stack_size(1000);

	HelpStruct help;
	help.syntax = "usr/search <equation-file> <border> [dump|nodump [split|left|dumb]]"
		"";
	help.description = "Computes all end configurations "
		"using split algorithm.";
	help.input = "Input grid in a format generated with ../ca/dump.";
	help.output = "All possible end configurations, including sccs";
	help.add_param("equation-file", "file containing the ca's equation");
	help.add_param("border", "state that, set on border, will be dead");
	help.add_param("dump|nodump", "whether to dump graph on exit/abort");
	help.add_param("split|left|dumb",
		"algorithm to use, default is split");

	MyProgram p;
	return p.run(argc, argv, &help);
}

