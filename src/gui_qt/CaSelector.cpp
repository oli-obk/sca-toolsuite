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
#include "ca_eqs.h"
#include "equation_solver.h"

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

	try
	{
		eqsolver::expression_ast ast;
		// TODO: parse only, not phrase?
		eqsolver::build_tree(input_edit.widget().text().toAscii().data(), &ast);
		eqsolver::build_tree(formula_edit.widget().toPlainText().toAscii().data(), &ast);
	}
	catch(std::string err)
	{
		QMessageBox::critical(this, "Invalid ca.", QString::fromStdString(err));
		ok = false;
	}


	if(ok)
		accept();
//	else
//	 QMessageBox::critical(this, "Invalid ca.", "Is your formula wrong?");
}

CaSelector::CaSelector(QWidget *parent) :
	QDialog(parent),
	ca_type_edit("ca type"),
	formula_edit("ca formula:"),
	input_edit("input formula:"),
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
	ca_type_edit.widget().setDisabled(true);

	vbox_main.addLayout(&ca_type_edit.layout());
	vbox_main.addLayout(&formula_edit.layout());
	vbox_main.addLayout(&input_edit.layout());
	vbox_main.addWidget(button_box);
	setLayout(&vbox_main);

	connect(button_box, SIGNAL(accepted()), this, SLOT(try_accept()));
	connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));
}

sca::ca::input_ca* CaSelector::instantiate_ca()
{
	// TODO: redundant in DrawArea.cpp
	using eq_sim_t = sca::ca::simulator_t<sca::ca::eqsolver_t,
		def_coord_traits, def_cell_traits>;
	return new eq_sim_t(
		formula_edit.widget().toPlainText().toAscii().data(),
		input_edit.widget().text().toAscii().data()
		);
}
