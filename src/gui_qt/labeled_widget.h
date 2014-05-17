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

#ifndef LABELED_WIDGET_H
#define LABELED_WIDGET_H

#include <QLabel>
#include <QHBoxLayout>

template<class QtWidget>
class LabeledWidget
{
	QLabel lbl;
	QtWidget _widget;
	QHBoxLayout _layout;

public:
	QHBoxLayout& layout() { return _layout; }
	const QHBoxLayout& layout() const { return _layout; }
	QtWidget& widget() { return _widget; }
	const QtWidget& widget() const { return _widget; }

	LabeledWidget(const char* text) :
		lbl(text)
	{
		_layout.addWidget(&lbl);
		_layout.addWidget(&_widget, 1);
	}
};

#endif // LABELED_WIDGET_H
