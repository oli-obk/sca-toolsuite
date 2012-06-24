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



