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

#include <QVector>
#include <QStatusBar>
#include "MainWindow.h"
#include "CaSelector.h"

void MainWindow::setup_ui()
{
	/*
		MENU BAR
	*/
	setMenuBar(&menu_bar);

	/*
		TOOL BAR
	*/
//	tool_bar.setIconSize(QSize(24, 24));
//	addToolBar(Qt::TopToolBarArea, &tool_bar);

	/*
		CENTRAL WIDGET
	*/
	setCentralWidget(&central_widget);
	hbox_main.addLayout(&vbox_left, 1);
	hbox_main.addLayout(&vbox_right);

	vbox_left.addWidget(&draw_area, 0);
	vbox_right.addWidget(&spacer, 1);
	vbox_right.addWidget(&btn_run);
	vbox_right.addWidget(&btn_step);
	vbox_right.addLayout(&pixel_size_chooser.layout());
	vbox_right.addLayout(&time_interval_chooser.layout());
	ca_type_edit.widget().setText("Select");
	vbox_right.addLayout(&ca_type_edit.layout());
	btn_run.setCheckable(true);

	draw_area.fill_grid();
	draw_area.set_pixel_size(pixel_size_chooser.widget().value());

	connect(&time_interval_chooser.widget(), SIGNAL(valueChanged(int)),
		&draw_area, SLOT(set_timeout_interval(int)));
	connect(&pixel_size_chooser.widget(), SIGNAL(valueChanged(int)),
		this, SLOT(change_pixel_size(int)));
	connect(&state_machine, SIGNAL(updated(StateMachine::STATE)),
		this, SLOT(state_updated(StateMachine::STATE)));
	connect(&btn_step, SIGNAL(released()),
		&state_machine, SLOT(trigger_step()));
	connect(&btn_run, SIGNAL(released()),
		&state_machine, SLOT(trigger_pause()));
	connect(&ca_type_edit.widget(), SIGNAL(clicked()),
		this, SLOT(change_ca_type()));

	/*pixel_size_chooser.widget().setMinimum(1);
	pixel_size_chooser.widget().setMaximum(255);
	pixel_size_chooser.widget().setValue(4);
	time_interval_chooser.widget().setMinimum(0);
	time_interval_chooser.widget().setMaximum(1000);
	time_interval_chooser.widget().setValue(10);*/
}

void MainWindow::retranslate_ui()
{
	btn_run.setText("Pause");
	btn_step.setText("Step");
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	menu_bar(this),
//	tool_bar(this),
	central_widget(this),
	hbox_main(&central_widget),
	draw_area(state_machine),
	pixel_size_chooser("UI size: "),
	time_interval_chooser("Step time: "),
	/*ca_type_chooser("CA input type: "),
	ca_formula_edit("CA formula: ")*/
	ca_type_edit("")
{
	setup_ui();
	retranslate_ui();
	state_machine.set(StateMachine::STATE_WELCOME, true);
}

void MainWindow::slot_help_about()
{
	QMessageBox::about ( NULL, "About",
			     "<h1>Qt GUI for sca-toolsuite</h1>"
			     "<i>(c) 2012-2012</i><br/>"
			     "by Johannes Lorenz<br/><br/>"
			     "<a href=\"https://github.com/JohannesLorenz/sca-toolsuite\">https://github.com/JohannesLorenz/sca-toolsuite</a>");

}

void MainWindow::slot_help_about_qt() {
	QMessageBox::aboutQt ( NULL, tr("About - Qt") );
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

	menu_bar.state_updated(new_state);

	statusBar()->showMessage(state_machine.status_msg());
}

void MainWindow::change_pixel_size(int new_size)
{
	draw_area.set_pixel_size(new_size);
}

void MainWindow::change_ca_type()
{
	CaSelector ca_sel(this);
	ca_sel.setModal(true);
	ca_sel.exec();
}

