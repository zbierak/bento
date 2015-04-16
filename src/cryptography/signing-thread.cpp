/*
 * signing-thread.cpp
 *
 *  Created on: Apr 16, 2015
 *      Author: z
 */

#include "signing-thread.h"
#include "micro-protocol.h"

#include "../zmq-wrappers/zmq-inproc.h"
#include "../zmq-wrappers/zmq-helpers.h"

#include <boost/lexical_cast.hpp>

using namespace std;

namespace bento {

// ------ a signing thread (not to be confused with a singing thread though)

SigningThread::SigningThread(JobQueue<CryptoJob*>* queue, IMessageSigner* signer, const Topology& topology, const std::string& cryptoManagerSocketName) :
		m_topology(topology)
{
	m_queue= queue;
	m_signer = signer;
	m_running = false;
	m_thread = NULL;

	m_returnChan = new zmq::socket_t(Context::getInstance(), ZMQ_DEALER);
	m_returnChan->connect(("inproc://"+cryptoManagerSocketName).c_str());
}

SigningThread::~SigningThread()
{
	delete m_returnChan;
	delete m_thread;
}

void SigningThread::start()
{
	m_running = true;
	m_thread = new boost::thread(boost::bind(&SigningThread::run, this));
}

void SigningThread::stop()
{
	m_running = false;
}

void SigningThread::join()
{
	m_thread->join();
}

void SigningThread::run()
{
	CryptoJob* nextJob = NULL;

	while (m_running)
	{
		if (m_queue->pop(nextJob))
		{
			if (nextJob->jobType == CryptoJob::CJ_SIGN)
			{
				string signature;
				m_signer->signMessage(&m_topology, nextJob->node, nextJob->msgType, nextJob->msg, signature);

				// outgoing message format: target (string), type (int32_t), msg (string), signature (string)
				zmqSend(m_returnChan, boost::lexical_cast<string, int32_t>(CMT_SIGN_RESPONSE), true);
				zmqSend(m_returnChan, nextJob->node, true);
				zmqSend(m_returnChan, boost::lexical_cast<string>(nextJob->msgType), true);
				zmqSend(m_returnChan, nextJob->msg, true);
				zmqSend(m_returnChan, signature);
			}
			else if (nextJob->jobType == CryptoJob::CJ_VERIFY)
			{
				bool correct = m_signer->verifyMessage(&m_topology, nextJob->node, nextJob->msgType, nextJob->msg, nextJob->signature);

				// outgoing message format: from (string), type (int32_t), msg (string), valid (bool)
				zmqSend(m_returnChan, boost::lexical_cast<string, int32_t>(CMT_VERIFY_RESPONSE), true);
				zmqSend(m_returnChan, nextJob->node, true);
				zmqSend(m_returnChan, boost::lexical_cast<string>(nextJob->msgType), true);
				zmqSend(m_returnChan, nextJob->msg, true);
				zmqSend(m_returnChan, boost::lexical_cast<string, bool>(correct));
			}

			delete nextJob;
		}
	}
}

}

