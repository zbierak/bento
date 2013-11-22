/*
 * timer-manager.h
 *
 *  Created on: Nov 22, 2013
 *      Author: zbierak
 */

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_

#include <set>

#include "timer-event.h"

namespace bento {

class TimerManager
{
public:
	TimerManager();
	virtual ~TimerManager();

	int setTimeout(unsigned timeout, const TimerEvent::TimeoutCallback& callback);
	bool cancelTimeout(unsigned timeoutId);

	int processReady();
private:
	typedef std::multiset<TimerEvent, TimerEvent::Comparator> EventMap;
	EventMap m_events;

	int m_nextEventId;
};

} /* namespace bento */

#endif /* TIMER_MANAGER_H_ */
