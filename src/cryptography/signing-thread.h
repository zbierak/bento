/*
 * signing-thread.h
 *
 *  Created on: Apr 16, 2015
 *      Author: z
 */

#ifndef SIGNING_THREAD_H_
#define SIGNING_THREAD_H_

#include <boost/thread.hpp>

#include "../message-signer.h"
#include "../topology/topology.h"

#include "../zmq-wrappers/zmq-context.h"

#include "job-queue.h"
#include "crypto-job.h"

namespace bento {

class SigningThread
{
public:
	SigningThread(JobQueue<CryptoJob>* queue, IMessageSigner* signer, const Topology& topology, const std::string& cryptoManagerSocketName);
	~SigningThread();

	void start();
	void stop();
	void join();

private:
	void run();

	JobQueue<CryptoJob>* m_queue;
	IMessageSigner* m_signer;

	const Topology& m_topology;

	boost::thread* m_thread;
	bool m_running;

	zmq::socket_t* m_returnChan;
};

}

#endif /* SIGNING_THREAD_H_ */
