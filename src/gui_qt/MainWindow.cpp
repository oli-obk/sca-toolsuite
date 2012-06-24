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

#include <QVector>
#include <QStatusBar>
#include "MainWindow.h"

void MainWindow::retranslate_ui()
{
	btn_run.setText("Pause");
	btn_step.setText("Step");
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	menu_bar(this),
	tool_bar(this),
	central_widget(this),
	hbox_main(&central_widget),
	draw_area(state_machine)
{
	/*
		MENU BAR
	*/
	setMenuBar(&menu_bar);

	/*
		TOOL BAR
	*/
	tool_bar.setIconSize(QSize(24, 24));
	addToolBar(Qt::TopToolBarArea, &tool_bar);

	/*
		CENTRAL WIDGET
	*/
	setCentralWidget(&central_widget);
	hbox_main.addLayout(&vbox_left, 0);
	hbox_main.addLayout(&vbox_right, 1);

	vbox_left.addStretch(1);
	vbox_left.addWidget(&draw_area, 0);
	vbox_left.addStretch(1);
	vbox_right.addWidget(&btn_run);
	vbox_right.addWidget(&btn_step);
	vbox_right.addWidget(&pixel_size_chooser);
	btn_run.setCheckable(true);

	pixel_size_chooser.setMinimum(1);
	pixel_size_chooser.setMaximum(255);
	pixel_size_chooser.setValue(4);

	draw_area.fill_grid();
	draw_area.set_pixel_size(pixel_size_chooser.value());

	connect(&pixel_size_chooser, SIGNAL(valueChanged(int)),
		this, SLOT(change_pixel_size(int)));
	connect(&state_machine, SIGNAL(updated(StateMachine::STATE)),
		this, SLOT(state_updated(StateMachine::STATE)));
	connect(&btn_step, SIGNAL(released()),
		&state_machine, SLOT(trigger_step()));
	connect(&btn_run, SIGNAL(released()),
		&state_machine, SLOT(trigger_pause()));

	retranslate_ui();

	state_machine.set(StateMachine::STATE_WELCOME, true);
}

void MainWindow::state_updated(StateMachine::STATE new_state)
{
	switch (new_state)
	{
		case StateMachine::STATE_INSTABLE:
			btn_step.setEnabled(true);
			break;
		default:
			btn_step.setDisabled(true);
			break;
	}

	statusBar()->showMessage(state_machine.status_msg());
}

