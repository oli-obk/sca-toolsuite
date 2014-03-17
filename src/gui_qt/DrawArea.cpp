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
#include "asm_basic.h"
#include "DrawArea.h"

DrawArea::DrawArea(StateMachine& _state_machine, QWidget *parent) :
	QLabel(parent),
	state_machine(_state_machine),
	grid_image(NULL),
	pixel_factor(1),
	min_color(0,255,0),
	max_color(255,0,0),
	next_cell(0)
{
	connect(&next_fire_timer, SIGNAL(timeout()),
		this, SLOT(slot_timeout()));
	connect(&state_machine, SIGNAL(updated(StateMachine::STATE)),
		this, SLOT(state_updated(StateMachine::STATE)));
}

void DrawArea::increase_cell(int x, int y, int steps)
{
	unsigned int coord = (y*dim.width)+x;

	const int new_value = sim_grid[coord] + steps;
	sim_grid[coord] = new_value;

	if(!is_border(&dim, coord))
	 grid_image->setPixel(x, y, color_of(new_value));

	update_pixmap();
}

void DrawArea::fire_cell(int coords)
{
	const int x = coords % dim.width;
	const int y = coords / dim.width;

	increase_cell(x, y, -4);
	increase_cell(x, y-1, 1);
	increase_cell(x-1, y, 1);
	increase_cell(x+1, y, 1);
	increase_cell(x, y+1, 1);
}

void DrawArea::slot_timeout()
{
	if(next_cell==0 || next_cell == container->size())
	{
		next_cell = 0;
		container->flush();
		if(calc_grid[current_hint]>2)
		{
			avalanche_1d_hint_noflush(&calc_grid, &dim,
				current_hint, container, NULL);
		}
		else {
			calc_grid[current_hint]++;
			next_fire_timer.stop();
			delete container;
			assert(sim_grid == calc_grid);
			state_machine.set(StateMachine::STATE_STABLE);
			return;
		}
	}
	fire_cell(container->data()[next_cell]);
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
		int coords = y * dim.width + x;

		increase_cell(x, y, 1);
		calc_grid[coords]++;

		if( calc_grid[coords] == 4)
		{
			/*if(state == StateMachine::STATE_STABLE_PAUSED) {
				state_machine.set(StateMachine::STATE_INSTABLE);
			}
			else*/

			//state_machine.set(StateMachine::STATE_SIMULATING);
			state_machine.trigger_throw();

			current_hint = coords;
			calc_grid[current_hint]--;
			container = new ArrayQueueNoFile(dim.area());

			if(state_machine.get() != StateMachine::STATE_INSTABLE)
			 next_fire_timer.start();
		}
	}
}

void DrawArea::fill_grid(FILE* fp)
{
	read_grid(fp, &calc_grid, &dim);
	// TODO: progress dialog here?
	stabilize(&calc_grid, &dim); // to keep invariant
	sim_grid = calc_grid;

	ColorTable tmp_ct(min_color, max_color, 0, 7);
	int entry = 0;
	for(ColorTable::const_iterator itr(tmp_ct); itr.valid();
		++itr, ++entry)
	 itr->to_32bit((int*)(color_table + entry));

	delete grid_image;
	grid_image = new QImage(dim.width, dim.height, QImage::Format_ARGB32);

	for(unsigned int y = 0; y<dim.height; y++)
	for(unsigned int x = 0; x<dim.width; x++)
	{
		if(x==0||x==dim.width-1||y==0||y==dim.height-1)
		 grid_image->setPixel(x,y, 9);
		else
		{
			unsigned int coord = (y*dim.width)+x;
			grid_image->setPixel(x, y, color_of(sim_grid[coord]));
		}
	}

	update_pixmap();
}

