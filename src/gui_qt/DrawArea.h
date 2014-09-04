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

#ifndef DRAWAREA_H
#define DRAWAREA_H

#include <iostream>
#include <QLabel>
#include <QTimer>
#include <QGridLayout>

#include "geometry.h"
#include "StateMachine.h"
#include "labeled_widget.h"
#include "image.h"

namespace sca { namespace ca {
	class input_ca;
} }

/*namespace sandpile
{
	template<class T>
	class _array_queue_no_file;
}*/

//! This will be hold for every cell
class ImgContainer : public QLabel
{
	Q_OBJECT
	const QPixmap* pixmap;
	point coords;
	void update_pixmap() {
		QLabel::setPixmap(pixmap->scaled(
			width(),
			height(), Qt::KeepAspectRatio));
	}

	void mousePressEvent(QMouseEvent *) {
		emit clicked(coords);
	}
public:
	ImgContainer(QWidget* parent, const point& coords) :
		QLabel(parent), coords(coords) {}
	void setPixmap ( const QPixmap * _pixmap ) {
		pixmap = _pixmap;
		update_pixmap();
	}
	void resizeEvent(QResizeEvent *) {
		update_pixmap();
	}
signals:
	void clicked(point);
};

//! This widget displays the grid
class DrawArea : public QWidget
{
	Q_OBJECT

	StateMachine& state_machine;

	int pixel_factor;
	int TIMER_INTERVAL;

	rgb min_color, max_color;

	QRgb color_table[9]; // 0 to 7 + sentinel
	inline const QPixmap& pixmap_of(int state) const {
		return pixmap_table[std::min((unsigned)state, 8u)];
	}

	QVector<QPixmap> pixmap_table;
	QVector<ImgContainer*> labels;

	void increase_cell(const point& coord, int steps);
	void fire_cell(int coords);

//	unsigned int next_cell;
//	int current_hint;
//	point current_hint;
//	sandpile::_array_queue_no_file<int*>* container;
	QTimer next_fire_timer;
	sca::ca::input_ca* ca = nullptr;

	std::vector<point> recent_active_cells;

	QGridLayout grid_layout;
	//QVector<QImage> imgs;

private slots:
	void slot_timeout();

	//! updates the whole pixmaps from the whole grid
	void update_pixmap();
	void state_updated(StateMachine::STATE new_state);

	void onMousePressed(point coords);

public:
	explicit DrawArea(StateMachine& _state_machine,
		const char* ca_eq,
		const char* input_eq,
		QWidget *parent = 0);
	~DrawArea();

	void set_pixel_size(int pixel_size);
	void fill_grid(std::istream& inf = std::cin);
	void reset_ca(sca::ca::input_ca* new_ca);

signals:
public slots:
	void set_timeout_interval(int msecs);
};

#endif // DRAWAREA_H
