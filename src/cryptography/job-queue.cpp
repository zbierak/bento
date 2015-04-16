/*
 * job-queue.cpp
 *
 *  Created on: Apr 16, 2015
 *      Author: z
 */

#include "job-queue.h"

namespace bento {

template <typename T>
JobQueue<T>::JobQueue()
{
	do_abort = false;
}

template <typename T>
void JobQueue<T>::push(const T& value)
{
	boost::mutex::scoped_lock lock(m);
	q.push(value);
	lock.unlock();
	v.notify_one();
}

template <typename T>
bool JobQueue<T>::pop(T& value)
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

template <typename T>
void JobQueue<T>::abort()
{
	boost::mutex::scoped_lock lock(m);
	do_abort = true;
	v.notify_all();
}

template <typename T>
bool JobQueue<T>::empty() const
{
	boost::mutex::scoped_lock lock(m);
	return q.empty();
}

}
