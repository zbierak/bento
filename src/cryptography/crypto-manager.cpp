/*
 * CryptoManager.cpp
 *
 *  Created on: Jun 4, 2014
 *      Author: zbierak
 */

#include "crypto-manager.h"
#include "micro-protocol.h"

#include "../zmq-wrappers/zmq-context.h"
#include "../zmq-wrappers/zmq-inproc.h"
#include "../zmq-wrappers/zmq-helpers.h"

#include "../logger.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

using namespace std;

namespace bento {

const std::string CRYPTO_MANAGER_NAME = "cryptography-manager";

CryptoManager::CryptoManager(const Topology& topology):
		m_workers(NULL),
		m_topology(topology)
{
	m_chanAtNode = new zmq::socket_t(Context::getInstance(), ZMQ_ROUTER);
	m_chanAtNode->bind(("inproc://"+CRYPTO_MANAGER_NAME).c_str());

	m_incoming = new JobQueue<CryptoJob*>();
	m_outgoing = new JobQueue<CryptoJob*>();

}

CryptoManager::~CryptoManager()
{
	// stop all workers, if not stopped already
	this->stop();

	delete m_chanAtNode;

	while (!m_incoming->empty())
	{
		CryptoJob* job = NULL;
		if (m_incoming->pop(job))
			free(job);
	}
	delete m_incoming;

	while (!m_outgoing->empty())
	{
		CryptoJob* job = NULL;
		if (m_outgoing->pop(job))
			free(job);
	}
	delete m_outgoing;
}

void CryptoManager::setMessageSigners(const std::vector<IMessageSigner*>& signers)
{
	if (m_workers != NULL)
	{
		LOG_ERROR("Signers cannot be changed once the node is running");
		return;
	}

	m_signers = signers;
}

std::vector<IMessageSigner*> CryptoManager::getMessageSigners()
{
	return m_signers;
}

void CryptoManager::start()
{
	// we only start worker threads if some signers have been passed using setMessageSigners()
	if (m_signers.size() > 0)
	{
		m_workers = new SigningThread*[m_signers.size()];
		for (unsigned i=0; i<m_signers.size(); i++)
		{
			m_workers[i] = new SigningThread(m_incoming, m_outgoing, m_signers[i], m_topology, CRYPTO_MANAGER_NAME);
			m_workers[i]->start();
		}
	}
}

void CryptoManager::stop()
{
	if (m_workers == NULL)
		return;

	// tell all workers to exit (if they are not sleeping)
	for (unsigned i=0; i<m_signers.size(); i++)
		m_workers[i]->stop();

	// notify all workers (if they are sleeping)
	m_incoming->abort();

	// wait for all workers to finish leaving their thread function
	for (unsigned i=0; i<m_signers.size(); i++)
		m_workers[i]->join();

	// and some cleanup
	for (unsigned i=0; i<m_signers.size(); i++)
		delete m_workers[i];
	delete[] m_workers;
	m_workers = NULL;
}


void CryptoManager::requestSign(const std::string& target, const int32_t type, const std::string& msg)
{
	CryptoJob* job = new CryptoJob();
	job->initSign(target, type, msg);
	m_incoming->push(job);
}

void CryptoManager::requestVerify(const std::string& from, const int32_t type, const std::string& msg, const std::string& signature)
{
	CryptoJob* job = new CryptoJob();
	job->initVerify(from, type, msg, signature);
	m_incoming->push(job);
}

CryptoJob* CryptoManager::getResponse()
{
	CryptoJob* job = NULL;
	if (m_workers == NULL || m_outgoing->empty() || !m_outgoing->pop(job))
		return NULL;

	return job;
}

} /* namespace bento */
