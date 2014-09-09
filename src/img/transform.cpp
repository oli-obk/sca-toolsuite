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

#include <algorithm>
#include <iostream>
#include <Magick++.h>
#include <magick/methods.h>
#include "general.h"
#include "geometry.h"
#include "ca.h"
#include "ca_eqs.h"

std::vector<char> get_file_contents(std::istream& stream = std::cin)
{
	int RD_BUF_SIZE = 128;
	char rdbuf[RD_BUF_SIZE];
	std::vector<char> contents;
	bool eof_hit;
	do
	{
		eof_hit = ! stream.read(rdbuf, RD_BUF_SIZE);
		contents.insert(contents.end(),
			rdbuf, rdbuf + stream.gcount());
	} while(!eof_hit) ;
	return contents;
}

class MyProgram : public Program
{
	exit_t main()
	{
		const char *equation = "v";
		std::string format = "ARGB";
		int iterations = 1;

		MagickCore::MagickCoreGenesis(*argv, Magick::MagickFalse);

		switch(argc)
		{
			case 4:
				iterations = atoi(argv[3]);
			case 3:
				format = argv[2];
				assert_always(format.length()==4, "Format must consist of 4 chars.");
				// TODO: only RGBACYMK allowed
			case 2:
				equation = argv[1];
				break;
			case 1:
			default:
				exit_usage();
		}
		// big endian -> reverse format for user
		std::reverse(format.begin(), format.end());

		std::vector<char> content = get_file_contents();

		try {

			// this is all much copying,
			// but it seems to be the only way...
			Magick::Blob blob(content.data(), content.size());
			Magick::Image img(blob);

			const dimension dim(img.size().width(),
				img.size().height());
			grid_t grid(dimension(dim.height(), dim.width()), 0);
			img.write(0, 0, dim.width(), dim.height(), format,
				Magick::CharPixel, grid.data().data());


			using ca_sim_t = sca::ca::simulator_t<
				sca::ca::eqsolver_t, def_coord_traits, def_cell_traits>;
			ca_sim_t sim(equation);
			sim.grid() = grid;
			sim.finalize();


			for(int i = 0; i < iterations; ++i)
			 sim.run_once(ca_sim_t::synchronous());

			Magick::Blob blob2;
			Magick::Image img2(dim.width(), dim.height(), format,
				Magick::CharPixel, sim.grid().data().data());
			// needed, otherwise we write "format-less":
			img2.magick(img.magick());
			img2.write(&blob2);

			std::cout.write(reinterpret_cast<const char*>(blob2.data()), blob2.length());
		}
		catch ( Magick::Exception & error) {
			std::cerr << "Caught Magick++ exception: "
				<< error.what() << std::endl;
		}

		return exit_t::success;
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "img/transform <equation> [<format> [<iterations>]]";
	help.description = "Transforms given image using a CA\n";
	help.input = "input image";
	help.output = "output image";
	help.add_param("<equation>", "transformation equation");
	help.add_param("<format>", "format string, like ARGB");
	help.add_param("<iterations>", "number of subsequent iterations");
	MyProgram p;
	return p.run(argc, argv, &help);
}



