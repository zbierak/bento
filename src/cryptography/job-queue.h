/*
 * job-queue.h
 *
 *  Created on: Apr 16, 2015
 *      Author: z
 */

#ifndef JOB_QUEUE_H_
#define JOB_QUEUE_H_

#include <boost/thread.hpp>
#include <queue>

namespace bento {

/**
 * Synchronized queue with multiple producers / consumers
 */
template <typename T>
class JobQueue
{
public:
	JobQueue()
	{
		do_abort = false;
	}

	void push(const T& value)
	{
		boost::mutex::scoped_lock lock(m);
		q.push(value);
		lock.unlock();
		v.notify_one();
	}

	bool pop(T& value)
	{
		boost::mutex::scoped_lock lock(m);
		while (q.empty())
		{
			if (do_abort) return false;
			v.wait(lock);
		}

		value = q.front();
		q.pop();
		return true;
	}

	void abort()
	{
		boost::mutex::scoped_lock lock(m);
		do_abort = true;
		v.notify_all();
	}

	bool empty() const
	{
		boost::mutex::scoped_lock lock(m);
		return q.empty();
	}

private:
	std::queue<T> q;
	mutable boost::mutex m;
	boost::condition_variable v;

	bool do_abort;
};

}

#endif /* JOB_QUEUE_H_ */
