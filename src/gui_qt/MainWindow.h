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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPushButton>
#include <QMenuBar>
#include <QToolBar>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QMessageBox>

#include "StateMachine.h"
#include "DrawArea.h"
#include "MenuBar.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

	StateMachine state_machine;

	MenuBar menu_bar;
	QToolBar tool_bar;
	QWidget central_widget;
	QHBoxLayout hbox_main;

	QVBoxLayout vbox_left;
	DrawArea draw_area;

	QVBoxLayout vbox_right;
	QPushButton btn_run, btn_step;
	QSpinBox pixel_size_chooser, time_interval_chooser;

	void retranslate_ui();

private slots:
	void state_updated(StateMachine::STATE new_state);
	inline void change_pixel_size(int new_size) {
		draw_area.set_pixel_size(new_size);
	}

public:
	explicit MainWindow(QWidget *parent = 0);

signals:

public slots:

	inline void slot_help_about ()
	{
		QMessageBox::about ( NULL, "About",
					"<h1>Qt GUI for sca-toolsuite</h1>"
					"<i>(c) 2012-2012</i><br/>"
					"by Johannes Lorenz<br/><br/>"
					"<a href=\"https://github.com/JohannesLorenz/sca-toolsuite\">https://github.com/JohannesLorenz/sca-toolsuite</a>");

	}

	inline void slot_help_about_qt () {
		QMessageBox::aboutQt ( NULL, tr("About - Qt") );
	}


};

#endif // MAINWINDOW_H
