/*
 * CryptoThread.cpp
 *
 *  Created on: Jun 4, 2014
 *      Author: zbierak
 */

#include "crypto-thread.h"

#include "zmq-wrappers/zmq-helpers.h"

#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <queue>

#include "logger.h"

using namespace std;

namespace bento {

const std::string CRYPTO_THREAD_NAME = "cryptography-thread";

// ------ message structure for holding sign/verify job

struct CryptoJob
{
	enum CryptoJobType { CJ_SIGN, CJ_VERIFY };

	CryptoJobType jobType;
	std::string node;
	int32_t msgType;
	std::string msg;
	std::string signature;

	void initSign(const std::string& node, const int32_t type, const std::string& msg)
	{
		this->jobType = CJ_SIGN;
		this->node = node;
		this->msgType = type;
		this->msg = msg;
	}

	void initVerify(const std::string& node, const int32_t type, const std::string& msg, const std::string& signature)
	{
		this->jobType = CJ_VERIFY;
		this->node = node;
		this->msgType = type;
		this->msg = msg;
		this->signature = signature;
	}
};

// ------ synchronized queue with multiple producers / consumers

class CryptoJobQueue
{
public:
	CryptoJobQueue()
	{
		do_abort = false;
	}

	void push(const CryptoJob& value)
	{
		boost::mutex::scoped_lock lock(m);
		q.push(value);
		lock.unlock();
		v.notify_one();
	}

	bool pop(CryptoJob& value)
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
	std::queue<CryptoJob> q;
	mutable boost::mutex m;
	boost::condition_variable v;

	bool do_abort;
};

// ------ a signing thread (not to be confused with a singing thread though)

class SigningThread
{
public:
	SigningThread(CryptoJobQueue* queue, IMessageSigner* signer, const Topology& topology) :
		m_topology(topology)
	{
		m_queue= queue;
		m_signer = signer;
		m_running = false;
		m_thread = NULL;

		m_returnChan = new zmq::socket_t(Context::getInstance(), ZMQ_DEALER);
		m_returnChan->connect(("inproc://"+CRYPTO_THREAD_NAME).c_str());
	}

	~SigningThread()
	{
		delete m_returnChan;
		delete m_thread;
	}

	void start()
	{
		m_running = true;
		m_thread = new boost::thread(boost::bind(&SigningThread::run, this));
	}

	void stop()
	{
		m_running = false;
	}

	void join()
	{
		m_thread->join();
	}

private:
	void run();

	CryptoJobQueue* m_queue;
	IMessageSigner* m_signer;

	const Topology& m_topology;

	boost::thread* m_thread;
	bool m_running;

	zmq::socket_t* m_returnChan;
};

void SigningThread::run()
{
	CryptoJob nextJob;

	while (m_running)
	{
		if (m_queue->pop(nextJob))
		{
			if (nextJob.jobType == CryptoJob::CJ_SIGN)
			{
				string signature;
				m_signer->signMessage(m_topology, nextJob.node, nextJob.msgType, nextJob.msg, signature);

				// outgoing message format: target (string), type (int32_t), msg (string), signature (string)
				zmqSend(m_returnChan, boost::lexical_cast<string, int32_t>(CMT_SIGN_RESPONSE), true);
				zmqSend(m_returnChan, nextJob.node, true);
				zmqSend(m_returnChan, boost::lexical_cast<string>(nextJob.msgType), true);
				zmqSend(m_returnChan, nextJob.msg, true);
				zmqSend(m_returnChan, signature);
			}
			else if (nextJob.jobType == CryptoJob::CJ_VERIFY)
			{
				bool correct = m_signer->verifyMessage(m_topology, nextJob.node, nextJob.msgType, nextJob.msg, nextJob.signature);

				// outgoing message format: from (string), type (int32_t), msg (string), valid (bool)
				zmqSend(m_returnChan, boost::lexical_cast<string, int32_t>(CMT_VERIFY_RESPONSE), true);
				zmqSend(m_returnChan, nextJob.node, true);
				zmqSend(m_returnChan, boost::lexical_cast<string>(nextJob.msgType), true);
				zmqSend(m_returnChan, nextJob.msg, true);
				zmqSend(m_returnChan, boost::lexical_cast<string, bool>(correct));
			}
		}
	}
}

// ------ implementation of the main crypto manager (it's not a thread anymore strictly speaking, the name is just for legacy purposes)

CryptoThread::CryptoThread(const Topology& topology):
		m_workers(NULL),
		m_topology(topology)
{
	m_chanAtNode = new zmq::socket_t(Context::getInstance(), ZMQ_ROUTER);
	m_chanAtNode->bind(("inproc://"+CRYPTO_THREAD_NAME).c_str());

	m_jobQueue = new CryptoJobQueue();

}

CryptoThread::~CryptoThread()
{
	//delete m_chanAtCrypto;
	delete m_chanAtNode;
	delete m_jobQueue;
}

void CryptoThread::setMessageSigners(const std::vector<IMessageSigner*>& signers)
{
	if (m_workers != NULL)
	{
		LOG_ERROR("Signers cannot be changed once the node is running");
		return;
	}

	m_signers = signers;
}

std::vector<IMessageSigner*> CryptoThread::getMessageSigners()
{
	return m_signers;
}

void CryptoThread::start()
{
	// we only start worker threads if some signers have been passed using setMessageSigners()
	if (m_signers.size() > 0)
	{
		m_workers = new SigningThread*[m_signers.size()];
		for (unsigned i=0; i<m_signers.size(); i++)
		{
			m_workers[i] = new SigningThread(m_jobQueue, m_signers[i], m_topology);
			m_workers[i]->start();
		}
	}
}

void CryptoThread::stop()
{
	// tell all workers to exit (if they are not sleeping)
	for (unsigned i=0; i<m_signers.size(); i++)
		m_workers[i]->stop();

	// notify all workers (if they are sleeping)
	m_jobQueue->abort();

	// wait for all workers to finish leaving their thread function
	for (unsigned i=0; i<m_signers.size(); i++)
		m_workers[i]->join();

	// and some cleanup
	for (unsigned i=0; i<m_signers.size(); i++)
		delete m_workers[i];
	delete[] m_workers;
}


void CryptoThread::requestSign(const std::string& target, const int32_t type, const std::string& msg)
{
	CryptoJob job;
	job.initSign(target, type, msg);
	m_jobQueue->push(job);
}

void CryptoThread::requestVerify(const std::string& from, const int32_t type, const std::string& msg, const std::string& signature)
{
	CryptoJob job;
	job.initVerify(from, type, msg, signature);
	m_jobQueue->push(job);
}

} /* namespace bento */
