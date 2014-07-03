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
//#include "asm_basic.h"
#include "DrawArea.h"

DrawArea::DrawArea(StateMachine& _state_machine, const char *ca_eq,
	const char *input_eq, QWidget *parent) :
	QWidget(parent),
	state_machine(_state_machine),
	pixel_factor(1),
	TIMER_INTERVAL(250),
	min_color(0,255,0),
	max_color(255,0,0),
	ca(new sca::ca::ca_simulator_t<sca::ca::ca_eqsolver_t, def_traits>(
		ca_eq, input_eq)),
	grid_layout(this)
{
	connect(&next_fire_timer, SIGNAL(timeout()),
		this, SLOT(slot_timeout()));
	connect(&state_machine, SIGNAL(updated(StateMachine::STATE)),
		this, SLOT(state_updated(StateMachine::STATE)));

	ColorTable tmp_ct(min_color, max_color, 0, 7);
	int entry = 0;

	for(ColorTable::const_iterator itr(tmp_ct); itr.valid();
		++itr, ++entry)
	{
		itr->to_32bit((int*)(color_table + entry));
	}
	color_table[8] = 0; // sentinel - black

	pixmap_table.resize(9);
	for(int i = 0; i < 9; ++i)
	{
		pixmap_table[i] = QPixmap(1, 1);
		pixmap_table[i].fill(color_table[i]);
	}

}

DrawArea::~DrawArea()
{
	// delete grid_image;
	for(int i = 0; i < labels.size(); ++i)
	 delete labels[i];

	// make program output
	std::cout << ca->grid();
	delete ca;
}

void DrawArea::set_pixel_size(int pixel_size) {
	pixel_factor = pixel_size;
	update_pixmap();
}

void DrawArea::slot_timeout()
{
	for(const point& p : recent_active_cells)
	{
		int coord = p.y * ca->grid().human_dim().width() + p.x;
		labels[coord]->setPixmap(&pixmap_of(ca->grid()[p]));
	}
	recent_active_cells = ca->active_cells();

	if(ca->has_active_cells())
	 ca->run_once();
	else {
		next_fire_timer.stop();
		state_machine.set(StateMachine::STATE_STABLE);
	}

	if(state_machine.get() == StateMachine::STATE_STEP)
	 state_machine.set(StateMachine::STATE_INSTABLE);
}

void DrawArea::update_pixmap()
{
	for(const point& p : ca->grid().points())
	{
		int coord = p.y * ca->grid().human_dim().width() + p.x;
		labels[coord]->setPixmap(&pixmap_of(ca->grid()[p]));
	}
}

void DrawArea::state_updated(StateMachine::STATE new_state)
{
	switch(new_state)
		{
		case StateMachine::STATE_STEP:
			QTimer::singleShot(TIMER_INTERVAL,
				this, SLOT(slot_timeout()));
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

void DrawArea::onMousePressed(point coords)
{
	StateMachine::STATE state = state_machine.get();

	if(state == StateMachine::STATE_STABLE
		|| state == StateMachine::STATE_WELCOME
		|| state == StateMachine::STATE_STABLE_PAUSED)
	{
		ca->input(coords);

		int coord = coords.y * ca->grid().human_dim().width() + coords.x;
		labels[coord]->setPixmap(&pixmap_of(ca->grid()[coords]));

		state_machine.trigger_throw();
		if(state_machine.get() != StateMachine::STATE_INSTABLE)
		{
			next_fire_timer.start();
		}
	}
}

void DrawArea::fill_grid(std::istream &inf)
{
	ca->grid() = grid_t(inf, ca->border_width());
	ca->finalize();

	labels.resize(ca->grid().human_dim().area());
	for(const point& p : ca->grid().points())
	{
		int idx = p.y * ca->grid().human_dim().width() + p.x;
		labels[idx] = new ImgContainer(parentWidget(), p);
		grid_layout.addWidget(labels[idx], p.y, p.x);
		QObject::connect(labels[idx], SIGNAL(clicked(point)),
			this, SLOT(onMousePressed(point)));
	}

	// TODO: redundant -> see reset_ca()
	update_pixmap();
	state_machine.trigger_throw();
	if(state_machine.get() != StateMachine::STATE_INSTABLE)
	{
		next_fire_timer.start();
	}
}

void DrawArea::reset_ca(sca::ca::input_ca *new_ca)
{
	grid_t grid_copy = std::move(ca->grid());
	delete ca;
	ca = new_ca;
	new_ca->grid() = std::move(grid_copy);
	new_ca->finalize();

	update_pixmap(); // TODO: useless?
	state_machine.trigger_throw();
	if(state_machine.get() != StateMachine::STATE_INSTABLE)
	{
		next_fire_timer.start();
	}
}

void DrawArea::set_timeout_interval(int msecs) {
	TIMER_INTERVAL = msecs;
	next_fire_timer.setInterval(TIMER_INTERVAL);
}

