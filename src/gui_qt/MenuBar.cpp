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

#include <QAction>
#include <QMenuBar>
#include "MenuBar.h"
#include "MainWindow.h"

void MenuBar::retranslate_ui()
{
	top_menus[MENU_FILE].setTitle(tr("Sandpile"));
	top_menus[MENU_TRANSFORM].setTitle(tr("Transform"));
	top_menus[MENU_MORE].setTitle(tr("More"));
	top_menus[MENU_HELP].setTitle(tr("Help"));

	actions[FILE_LOAD]->setText(tr("Load..."));
	actions[FILE_CREATE]->setText(tr("Create..."));
	actions[FILE_SAVE]->setText(tr("Save As..."));
	actions[FILE_EXPORT]->setText(tr("Export..."));
	actions[FILE_QUIT]->setText(tr("Quit"));
	actions[TRANSFORM_SUPER]->setText(tr("Make Superstable"));
	actions[TRANSFORM_BURNING]->setText(tr("Burning Test"));
	actions[MORE_RECURRENCE]->setText(tr("Recurrence Test"));
	actions[HELP_ABOUT]->setText(tr("About"));
	actions[HELP_ABOUTQT]->setText(tr("About Qt"));
}

MenuBar::MenuBar(MainWindow *parent) :
	QMenuBar(parent),
	main_window(parent)
{
	actions.resize(ACTION_SIZE);
	for(unsigned int i=0; i<MENU_SIZE; i++) {
		addMenu(&top_menus[i]);
	}

	append(MENU_FILE, FILE_LOAD, NULL);
	append(MENU_FILE, FILE_CREATE, NULL);
	append(MENU_FILE, FILE_SAVE, NULL);
	append(MENU_FILE, FILE_EXPORT, NULL);
	append(MENU_FILE, FILE_QUIT, SLOT(close()));
	append(MENU_TRANSFORM, TRANSFORM_SUPER, NULL);
	append(MENU_TRANSFORM, TRANSFORM_BURNING, NULL);
	append(MENU_MORE, MORE_RECURRENCE, NULL);
	append(MENU_HELP, HELP_ABOUT, SLOT(slot_help_about()));
	append(MENU_HELP, HELP_ABOUTQT, SLOT(slot_help_about_qt()));

	retranslate_ui();
}

QAction* MenuBar::append (enum MENU menu_no, enum ACTION action_no, const char* slot_name,
		const QKeySequence& short_key_sequence)
{
	QAction* new_action = new QAction(this);

	if(slot_name != NULL)
	 QObject::connect(new_action, SIGNAL(triggered()), main_window, slot_name);

	if(!short_key_sequence.isEmpty())
	 new_action->setShortcut(short_key_sequence);

	if(menu_no != MENU_SIZE)
	 top_menus[menu_no].addAction(new_action);

	actions[action_no] = new_action;
	return new_action;
}

void MenuBar::state_updated(StateMachine::STATE new_state)
{
	const bool simulating = (new_state == StateMachine::STATE_SIMULATING
		|| new_state == StateMachine::STATE_STEP);

	const bool instable = (simulating
		|| new_state == StateMachine::STATE_INSTABLE);

	actions[FILE_LOAD]->setDisabled(simulating);
	actions[FILE_CREATE]->setDisabled(simulating);
	actions[FILE_SAVE]->setDisabled(simulating);
	actions[FILE_EXPORT]->setDisabled(simulating);
	actions[FILE_QUIT]->setDisabled(simulating);
	actions[TRANSFORM_SUPER]->setDisabled(instable);
	actions[TRANSFORM_BURNING]->setDisabled(instable);
	actions[MORE_RECURRENCE]->setDisabled(instable);
}




