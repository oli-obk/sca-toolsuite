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

#include <QDialogButtonBox>
#include <QMessageBox>

#include "CaSelector.h"
#include "ca.h"

enum ca_source
{
	formula,
	file,
	custom_name
};

void CaSelector::try_accept()
{
	bool ok = true;

	/*switch(ca_type_edit.widget().currentIndex())
	{
		case formula:
			ca_obj = new ;

	}*/

	if(ok)
		accept();
	else
	 QMessageBox::critical(this, "Invalid ca.", "Is your formula wrong?");
}

CaSelector::CaSelector(QWidget *parent) :
	QDialog(parent),
	ca_type_edit("ca type"),
	formula_edit(""),
	button_box( new QDialogButtonBox(
		QDialogButtonBox::Ok |
		QDialogButtonBox::Cancel,
		Qt::Horizontal,
		this)
		)
{
	ca_type_edit.widget().addItem("formula");
	ca_type_edit.widget().addItem("file");
	ca_type_edit.widget().addItem("custom name");

	vbox_main.addLayout(&ca_type_edit.layout());
	vbox_main.addLayout(&formula_edit.layout());
	vbox_main.addWidget(button_box);
	setLayout(&vbox_main);

	connect(button_box, SIGNAL(accepted()), this, SLOT(try_accept()));
	connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));
}
