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

#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenu>
#include <QMenuBar>
#include <QVector>
#include "StateMachine.h"
class QAction;
class MainWindow;

class MenuBar : public QMenuBar
{
	Q_OBJECT

	enum MENU {
		MENU_FILE, /*MENU_TRANSFORM, MENU_MORE,*/
			MENU_VIEW, MENU_HELP, MENU_SIZE
	};

	enum ACTION {
		FILE_LOAD, FILE_CREATE, FILE_SAVE, FILE_EXPORT, FILE_QUIT,
		TRANSFORM_SUPER, TRANSFORM_BURNING,
		MORE_RECURRENCE,
		VIEW_FULLSCREEN,
		HELP_ABOUT, HELP_ABOUTQT,
		ACTION_SIZE
	};

	QMenu top_menus[MENU_SIZE];
	QVector<QAction*> actions;

	MainWindow* main_window;

	void retranslate_ui();
	QAction* append(enum MENU menu_no, enum ACTION action_no, const char* slot_name,
		const QKeySequence& short_key_sequence = QKeySequence());

private slots:
	inline void slot_fullscreen() { emit toggle_fullscreen(); }

public:
	void state_updated(StateMachine::STATE new_state);
	explicit MenuBar(MainWindow *parent = 0);

signals:
	void toggle_fullscreen();
};

#endif // MENUBAR_H
