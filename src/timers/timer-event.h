/*
 * timer-event.h
 *
 *  Created on: Nov 22, 2013
 *      Author: zbierak
 */

#ifndef TIMER_EVENT_H_
#define TIMER_EVENT_H_

#include <boost/function.hpp>

namespace bento {

class TimerEvent
{
public:
	typedef boost::function<void()> TimeoutCallback;

	TimerEvent(int id, unsigned timeout, const TimeoutCallback& callback);
	virtual ~TimerEvent();

	inline int getId() const { return m_id; }
	inline uint64_t getDeadline() const { return m_deadline; }
	inline void execute() const { m_callback(); }

	bool operator<(const TimerEvent& other) const;


	struct Comparator
	{
	    bool operator() (const TimerEvent& a, const TimerEvent& b) const
	    {
	        return a < b;
	    }
	};
private:
	int m_id;
	uint64_t m_deadline;
	TimeoutCallback m_callback;
};

} /* namespace bento */

#endif /* TIMER_EVENT_H_ */
