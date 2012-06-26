#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenu>
#include <QMenuBar>
#include <QVector>
#include "StateMachine.h"
struct QAction;
struct MainWindow;

class MenuBar : public QMenuBar
{
	Q_OBJECT

	enum MENU {
		MENU_FILE, MENU_TRANSFORM, MENU_MORE, MENU_HELP, MENU_SIZE
	};

	enum ACTION {
		FILE_LOAD, FILE_CREATE, FILE_SAVE, FILE_EXPORT, FILE_QUIT,
		TRANSFORM_SUPER, TRANSFORM_BURNING,
		MORE_RECURRENCE,
		HELP_ABOUT, HELP_ABOUTQT,
		ACTION_SIZE
	};

	QMenu top_menus[MENU_SIZE];
	QVector<QAction*> actions;

	MainWindow* main_window;

	void retranslate_ui();
	QAction* append(enum MENU menu_no, enum ACTION action_no, const char* slot_name,
		const QKeySequence& short_key_sequence = QKeySequence());

public:
	void state_updated(StateMachine::STATE new_state);
	explicit MenuBar(MainWindow *parent = 0);

signals:

public slots:

};

#endif // MENUBAR_H
