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
	JobQueue();

	void push(const T& value);
	bool pop(T& value);
	void abort();
	bool empty() const;

private:
	std::queue<T> q;
	mutable boost::mutex m;
	boost::condition_variable v;

	bool do_abort;
};

}

#endif /* JOB_QUEUE_H_ */
