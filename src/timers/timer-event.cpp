/*
 * timer-event.cpp
 *
 *  Created on: Nov 22, 2013
 *      Author: zbierak
 */

#include "timer-event.h"
#include "timer.h"

namespace bento {

TimerEvent::TimerEvent(int id, unsigned timeout, const TimeoutCallback& callback):
		m_id(id),
		m_callback(callback)
{
	m_deadline = Timer::now() + timeout;
}

TimerEvent::~TimerEvent() {}

bool TimerEvent::operator <(const TimerEvent& other) const
{
	return this->m_deadline < other.m_deadline;
}

} /* namespace bento */
