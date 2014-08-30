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

#include <QApplication>
#include "general.h"
#include "MainWindow.h"

class MyProgram : public Program
{
	exit_t main()
	{
		const char *ca_eq =
			"v+(-4*(v>=4))"
			"+(a[-1,0]>=4)+(a[0,-1]>=4)+(a[1,0]>=4)+(a[0,1]>=4)",
			*input_eq = "v+1";

		switch(argc)
		{
			case 3:
				input_eq = argv[2];
			case 2:
				ca_eq = argv[1];
			case 1:
				break;
			default:
				exit_usage();
		}

		QApplication app(argc, argv);
		MainWindow mainwindow(ca_eq, input_eq);
		mainwindow.show();

		return success(app.exec() == 0);
	}
};

int main(int argc, char** argv)
{
	HelpStruct help;
	help.syntax = "gui_qt/gui_qt [<ca_formula> [<input_formula>]]";
	help.description = "GUI to simulate CA.";
	help.input = "start grid for simulation";
	help.output = "end grid of simulation";
	help.add_param("ca_equation", "formula for the ca, default is ASM");
	help.add_param("input_formula", "formula for mouse click input"
		"default is `v+1'");

	MyProgram p;
	return p.run(argc, argv, &help);
}

