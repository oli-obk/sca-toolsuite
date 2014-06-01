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

#ifndef CASELECTOR_H
#define CASELECTOR_H

#include <QDialog>

#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QRadioButton>
#include <QComboBox>
class QDialogButtonBox;

#include "labeled_widget.h"

namespace sca { namespace ca {
	class input_ca;
}}

enum class ca_type_t
{
	formula,
	custom,
	invalid
};

struct ca_id
{
	ca_type_t ca_type;
	const char* name_or_type;
};

class CaSelector : public QDialog
{
	Q_OBJECT
	QVBoxLayout vbox_main;
	LabeledWidget<QComboBox> ca_type_edit;
	LabeledWidget<QTextEdit> formula_edit;
	LabeledWidget<QLineEdit> input_edit;
	QDialogButtonBox* button_box;

	//sca::ca::input_array* ca_obj;
	void get_ca_id();
private slots:
	void try_accept();

public:
	explicit CaSelector(QWidget *parent = nullptr);

	//! returns new allocated pointer. you need to delete it yourself
	sca::ca::input_ca* instantiate_ca();
};

#endif // CASELECTOR_H
