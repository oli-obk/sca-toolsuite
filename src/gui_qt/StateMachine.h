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

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QObject>

class StateMachine : public QObject
{
	Q_OBJECT
public:
	enum STATE
	{
		STATE_WELCOME, //!< welcome message, same as stable
		STATE_STABLE, //!< stable
		STATE_INSTABLE, //!< instable, but not simulating (i.e. paused)
		STATE_SIMULATING, //!< simulating right now
		STATE_STEP, //!< simulating right now, but only for one step
		STATE_STABLE_PAUSED //!< stable and paused
	};
	static const char* (msg[6]);

private:
	STATE state;
	void illegal_state() { exit(99); /* TODO... */ }
public:
	StateMachine();
	inline const char* status_msg() const { return msg[state]; }
	inline void set(STATE new_state, bool force = false)
	{
		if(force || state!=new_state) {
			state = new_state;
			emit updated(state);
		}
	}
	inline STATE get() const { return state; }
signals:
	void updated(StateMachine::STATE new_state);
public slots:
	inline void trigger_pause()
	{
		switch(state)
		{
			case STATE_INSTABLE: set(STATE_SIMULATING); break;
			case STATE_SIMULATING: set(STATE_INSTABLE); break;
			case STATE_STABLE:
			case STATE_WELCOME:
				set(STATE_STABLE_PAUSED); break;
			case STATE_STABLE_PAUSED: set(STATE_STABLE); break;
			default: illegal_state();
		}
	}
	inline void trigger_step() {
		if(state == STATE_INSTABLE) set(STATE_STEP);
		else illegal_state();
	}
	inline void trigger_throw() {
		if(state == STATE_STABLE_PAUSED) set(STATE_INSTABLE);
		else set(STATE_SIMULATING);
	}

};

#endif // STATEMACHINE_H
