/*
 * timer-manager.cpp
 *
 *  Created on: Nov 22, 2013
 *      Author: zbierak
 */

#include <iostream>

#include "timer-manager.h"
#include "timer.h"

namespace bento {

TimerManager::TimerManager() :
		m_nextEventId(1)
{
}

TimerManager::~TimerManager()
{
}

int TimerManager::setTimeout(unsigned timeout, const TimerEvent::TimeoutCallback& callback)
{
	m_events.insert(TimerEvent(m_nextEventId, timeout, callback));
	return m_nextEventId++;
}

bool TimerManager::cancelTimeout(unsigned timeoutId)
{
	for (EventMap::iterator it = m_events.begin(); it != m_events.end(); ++it)
	{
		if (it->getId() == timeoutId)
		{
			m_events.erase(it);
			return true;
		}
	}

	return false;
}

int TimerManager::processReady()
{
	uint64_t now;
	while (!m_events.empty())
	{
		now = Timer::Now::milliseconds();
		if (m_events.begin()->getDeadline() > now)
			break;

		m_events.begin()->execute();
		m_events.erase(m_events.begin());
	}

	if (m_events.empty())
		return -1;

	int timeLeft = static_cast<int>(m_events.begin()->getDeadline() - now);
	return timeLeft;
}

} /* namespace bento */
