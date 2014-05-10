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

#include "geometry.h"
#include "StateMachine.h"
#include "image.h"

namespace sandpile
{
	template<class T>
	class _array_queue_no_file;
}

class DrawArea : public QLabel
{
	Q_OBJECT

	StateMachine& state_machine;

	QImage* grid_image;
	int pixel_factor;
	int TIMER_INTERVAL;

/*	dimension dim;
	std::vector<int> sim_grid;
	std::vector<int> calc_grid;*/
	grid_t sim_grid, calc_grid;
	rgb min_color, max_color;

	QRgb color_table[8]; // 0 to 7
	inline QRgb color_of(int grains) const { return color_table[grains]; }

	void increase_cell(const point& coord, int steps);
	void fire_cell(int coords);

	unsigned int next_cell;
//	int current_hint;
	point current_hint;
	sandpile::_array_queue_no_file<int*>* container;
	QTimer next_fire_timer;

private slots:
	void slot_timeout();
	inline void update_pixmap() {
		setPixmap(QPixmap::fromImage(*grid_image).scaled(
			sim_grid.internal_dim().width()*pixel_factor,
			sim_grid.internal_dim().height()*pixel_factor));
	}
	void state_updated(StateMachine::STATE new_state);

public:
	explicit DrawArea(StateMachine& _state_machine, QWidget *parent = 0);
	inline ~DrawArea() {
		delete grid_image;
		std::cout << calc_grid;
	}
	inline void set_pixel_size(int pixel_size) {
		pixel_factor = pixel_size;
		update_pixmap();
	}
	void fill_grid(std::istream& inf = std::cin);

signals:
public slots:
	inline void set_timeout_interval(int msecs) {
		TIMER_INTERVAL = msecs;
		next_fire_timer.setInterval(TIMER_INTERVAL);
	}

protected:
	void mousePressEvent(QMouseEvent *event);
};

#endif // DRAWAREA_H
