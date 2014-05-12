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

#include <QProgressDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include "asm_basic.h"
#include "DrawArea.h"

DrawArea::DrawArea(StateMachine& _state_machine, QWidget *parent) :
//	QTableWidget(parent),
	QWidget(parent),
	state_machine(_state_machine),
	//grid_image(NULL),
	pixel_factor(1),
	sim_grid(1),
	calc_grid(1),
	min_color(0,255,0),
	max_color(255,0,0),
	next_cell(0),
	//table_widget(*this)
	grid_layout(this)
{
	connect(&next_fire_timer, SIGNAL(timeout()),
		this, SLOT(slot_timeout()));
	connect(&state_machine, SIGNAL(updated(StateMachine::STATE)),
		this, SLOT(state_updated(StateMachine::STATE)));
}

void DrawArea::increase_cell(const point& coord, int steps)
{
	const int new_value = sim_grid.at_internal(coord) + steps;
	sim_grid.at_internal(coord) = new_value;

	if(!calc_grid.point_is_on_border(coord))
	{
		//table_widget.item(coord.x, coord.y)->setBackgroundColor(color_of(new_value));
		QPalette pal(color_of(new_value));
		grid_layout.itemAtPosition(coord.x, coord.y)->widget()->setAutoFillBackground(true); // TODO: needed?
		grid_layout.itemAtPosition(coord.x, coord.y)->widget()->setPalette(pal);
	}
//	 grid_image->setPixel(coord.x, coord.y, color_of(new_value));

	update_pixmap();
}

void DrawArea::fire_cell(int coords)
{
	const int x = coords % calc_grid.internal_dim().width();
	const int y = coords / calc_grid.internal_dim().width();
	const point p(x, y);

	increase_cell(p, -4);
	increase_cell(p + point(0, -1), 1);
	increase_cell(p + point(-1, 0), 1);
	increase_cell(p + point(+1, 0), 1);
	increase_cell(p + point(0, +1), 1);
}

void DrawArea::slot_timeout()
{
	if(next_cell==0 || next_cell == container->size())
	{
		next_cell = 0;
		container->flush();
		if(calc_grid[current_hint]>2)
		{
			sandpile::avalanche_1d_hint_noflush_single(calc_grid,
				current_hint, *container);
		}
		else {
			calc_grid[current_hint]++;
			next_fire_timer.stop();
			delete container;
			assert(sim_grid.data() == calc_grid.data());
			state_machine.set(StateMachine::STATE_STABLE);
			return;
		}
	}
	fire_cell(container->data()[next_cell] - calc_grid.data().data() /* pointer difference */);
	next_cell++;

	if(state_machine.get() == StateMachine::STATE_STEP)
	 state_machine.set(StateMachine::STATE_INSTABLE);
}

void DrawArea::state_updated(StateMachine::STATE new_state)
{
	switch(new_state)
	{
		case StateMachine::STATE_STEP:
			QTimer::singleShot(TIMER_INTERVAL, this, SLOT(slot_timeout()));
			break;
		case StateMachine::STATE_INSTABLE:
			next_fire_timer.stop();
			break;
		case StateMachine::STATE_SIMULATING:
			next_fire_timer.start();
			break;
		default: break;
	}
}

void DrawArea::mousePressEvent(QMouseEvent *event)
{
	StateMachine::STATE state = state_machine.get();

	if (event->button() == Qt::LeftButton &&
		(state == StateMachine::STATE_STABLE
		|| state == StateMachine::STATE_WELCOME
		|| state == StateMachine::STATE_STABLE_PAUSED))
	{	
		int x = event->pos().x() / pixel_factor;
		int y = event->pos().y() / pixel_factor;
		//int coords = y * calc_grid.internal_dim().width() + x; // TODO: use point
		point coords(x, y);

		increase_cell(coords, 1);
		calc_grid.at_internal(coords)++;

		if( calc_grid.at_internal(coords) == 4)
		{
			/*if(state == StateMachine::STATE_STABLE_PAUSED) {
				state_machine.set(StateMachine::STATE_INSTABLE);
			}
			else*/

			//state_machine.set(StateMachine::STATE_SIMULATING);
			state_machine.trigger_throw();

			current_hint = coords;
			calc_grid.at_internal(current_hint)--;
			container = new sandpile::array_queue_no_file(calc_grid.internal_dim().area()); // TODO: human dim

			if(state_machine.get() != StateMachine::STATE_INSTABLE)
			 next_fire_timer.start();
		}
	}
}

void DrawArea::fill_grid(std::istream &inf)
{
//	read_grid(fp, &calc_grid, &dim);
	calc_grid = grid_t(inf, 1);

	// TODO: progress dialog here?
	sandpile::stabilize(calc_grid); // to keep invariant
	sim_grid = calc_grid;

	ColorTable tmp_ct(min_color, max_color, 0, 7);
	int entry = 0;
	for(ColorTable::const_iterator itr(tmp_ct); itr.valid();
		++itr, ++entry)
	 itr->to_32bit((int*)(color_table + entry));

/*	delete grid_image;
	grid_image = new QImage(calc_grid.internal_dim().width(),
		calc_grid.internal_dim().height(),
		QImage::Format_ARGB32);*/
	//table_widget.resize(calc_grid.internal_dim().width(), calc_grid.internal_dim().height());
	//table_widget.setRowCount(calc_grid.internal_dim().width());
	//table_widget.setColumnCount(calc_grid.internal_dim().height());
	// TODO

	for(const point& p : sim_grid.points())
	{
	//	std::cout << table_widget.size().width() << ", " << table_widget.size().height() << std::endl;
		std::cout << "grid image: " << sim_grid.internal_dim()
			<< ", p: " << p << std::endl;
		//table_widget.item(p.x, p.y)->setBackgroundColor(color_of(sim_grid[p]));
	//	table_widget.setItem(p.x, p.y, new QTableWidgetItem()); // this is not perfect...
	//	QRgb cof = color_of(sim_grid[p]); (void) cof;
	//	assert(table_widget.item(p.x, p.y));
	//	table_widget.item(p.x, p.y)->setBackgroundColor(color_of(sim_grid[p]));
		// TODO
	}
/*	for(unsigned int y = 0; y<dim.height(); y++)
	for(unsigned int x = 0; x<dim.width(); x++)
	{
		if(x==0||x==dim.width()-1||y==0||y==dim.height()-1)
		 grid_image->setPixel(x,y, 9);
		else
		{
			unsigned int coord = (y*dim.width())+x;
			grid_image->setPixel(x, y, color_of(sim_grid[coord]));
		}
	}*/
	std::cout << "int w h: " << sim_grid.internal_dim().width() << ", "
		<< sim_grid.internal_dim().height() << std::endl;
		std::cout << "int2 w h: " << calc_grid.internal_dim().width() << ", "
		<< calc_grid.internal_dim().height() << std::endl;

	update_pixmap();
}

