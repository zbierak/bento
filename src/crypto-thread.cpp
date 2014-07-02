/*
 * CryptoThread.cpp
 *
 *  Created on: Jun 4, 2014
 *      Author: zbierak
 */

#include "crypto-thread.h"

#include "zmq-wrappers/zmq-helpers.h"

#include <boost/lexical_cast.hpp>

#include "logger.h"

using namespace std;

namespace bento {

const std::string CRYPTO_THREAD_NAME = "cryptography-thread";

CryptoThread::CryptoThread(const Topology& topology):
		m_running(false),
		m_thread(NULL),
		m_signer(NULL),
		m_topology(topology)
{
	m_chanAtCrypto = new zmq::socket_t(Context::getInstance(), ZMQ_PAIR);
	m_chanAtNode = new zmq::socket_t(Context::getInstance(), ZMQ_PAIR);

	uint32_t hwm = 0;
	m_chanAtCrypto->setsockopt(ZMQ_SNDHWM, (const void*)&hwm, sizeof(hwm));
	m_chanAtNode->setsockopt(ZMQ_SNDHWM, (const void*)&hwm, sizeof(hwm));

	m_chanAtCrypto->bind(("inproc://"+CRYPTO_THREAD_NAME).c_str());
	m_chanAtNode->connect(("inproc://"+CRYPTO_THREAD_NAME).c_str());

}

CryptoThread::~CryptoThread()
{
	delete m_chanAtCrypto;
	delete m_chanAtNode;
}

void CryptoThread::setMessageSigner(IMessageSigner* signer)
{
	m_signer = signer;
}

void CryptoThread::start()
{
	m_running = true;
	m_thread = new boost::thread(boost::bind(&CryptoThread::run, this));
}

void CryptoThread::stop()
{
	if (m_running)
	{
		m_running = false;
		zmqSend(m_chanAtNode, boost::lexical_cast<string, int32_t>(CMT_END_OF_THE_WORLD));
		m_thread->join();
		delete m_thread;
	}
}

void CryptoThread::run()
{
	string msg;
	int32_t msgType;

	while (m_running)
	{
		zmqRecv(m_chanAtCrypto, msg);
		msgType = boost::lexical_cast<int32_t>(msg);

		switch (msgType)
		{
		case CMT_SIGN_REQUEST:
			LOG_DEBUG("Received a sign request");
			signMessage();
			break;
		case CMT_VERIFY_REQUEST:
			LOG_DEBUG("Received a verify request");
			verifyMessage();
			break;
		};
	}
}

void CryptoThread::signMessage()
{
	assert(m_signer != NULL);

	// incoming message format: target (string), type (int32_t), msg (string)
	string target, typeStr, msg, signature;
	int32_t type;

	zmqRecv(m_chanAtCrypto, target);
	zmqRecv(m_chanAtCrypto, typeStr);
	type = boost::lexical_cast<int32_t>(typeStr);
	zmqRecv(m_chanAtCrypto, msg);

	m_signer->signMessage(m_topology, target, type, msg, signature);

	// outgoing message format: target (string), type (int32_t), msg (string), signature (string)
	zmqSend(m_chanAtCrypto, boost::lexical_cast<string, int32_t>(CMT_SIGN_RESPONSE), true);
	zmqSend(m_chanAtCrypto, target, true);
	zmqSend(m_chanAtCrypto, typeStr, true);
	zmqSend(m_chanAtCrypto, msg, true);
	zmqSend(m_chanAtCrypto, signature);
}

void CryptoThread::verifyMessage()
{
	assert(m_signer != NULL);

	// incoming message format: from (string), type (int32_t), msg (string), signature (string)
	string from, typeStr, msg, signature;
	int32_t type;

	zmqRecv(m_chanAtCrypto, from);
	zmqRecv(m_chanAtCrypto, typeStr);
	type = boost::lexical_cast<int32_t>(typeStr);
	zmqRecv(m_chanAtCrypto, msg);
	zmqRecv(m_chanAtCrypto, signature);

	bool correct = m_signer->verifyMessage(m_topology, from, type, msg, signature);

	// outgoing message format: from (string), type (int32_t), msg (string), valid (bool)
	zmqSend(m_chanAtCrypto, boost::lexical_cast<string, int32_t>(CMT_VERIFY_RESPONSE), true);
	zmqSend(m_chanAtCrypto, from, true);
	zmqSend(m_chanAtCrypto, typeStr, true);
	zmqSend(m_chanAtCrypto, msg, true);
	zmqSend(m_chanAtCrypto, boost::lexical_cast<string, bool>(correct));
}

void CryptoThread::requestSign(const std::string& target, const int32_t type, const std::string& msg)
{
	LOG_DEBUG("Node has requested signing a message");

	// outgoing message format: target (string), type (int32_t), msg (string)
	zmqSend(m_chanAtNode, boost::lexical_cast<string, int32_t>(CMT_SIGN_REQUEST), true);
	zmqSend(m_chanAtNode, target, true);
	zmqSend(m_chanAtNode, boost::lexical_cast<string>(type), true);
	zmqSend(m_chanAtNode, msg);
}

void CryptoThread::requestVerify(const std::string& from, const int32_t type, const std::string& msg, const std::string& signature)
{
	LOG_DEBUG("Node has requested verifying a signature");

	// incoming message format: from (string), type (int32_t), msg (string), signature (string)
	zmqSend(m_chanAtNode, boost::lexical_cast<string, int32_t>(CMT_VERIFY_REQUEST), true);
	zmqSend(m_chanAtNode, from, true);
	zmqSend(m_chanAtNode, boost::lexical_cast<string>(type), true);
	zmqSend(m_chanAtNode, msg, true);
	zmqSend(m_chanAtNode, signature);
}

} /* namespace bento */
