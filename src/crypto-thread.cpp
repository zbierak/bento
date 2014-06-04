/*
 * CryptoThread.cpp
 *
 *  Created on: Jun 4, 2014
 *      Author: zbierak
 */

#include "crypto-thread.h"

#include <boost/lexical_cast.hpp>

#include "logger.h"

using namespace std;

namespace bento {

const std::string CRYPTO_THREAD_NAME = "cryptography-thread";

CryptoThread::CryptoThread(const Topology& topology):
		m_running(false),
		m_thread(NULL),
		m_signer(NULL),
		m_topology(topology),
		m_chanAtCrypto(CRYPTO_THREAD_NAME),
		m_chanAtNode(CRYPTO_THREAD_NAME)
{
}

CryptoThread::~CryptoThread()
{
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
		m_chanAtNode.send(boost::lexical_cast<string, int32_t>(CMT_END_OF_THE_WORLD));
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
		m_chanAtCrypto.recv(msg);
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

	m_chanAtCrypto.recv(target);
	m_chanAtCrypto.recv(typeStr);
	type = boost::lexical_cast<int32_t>(typeStr);
	m_chanAtCrypto.recv(msg);

	m_signer->signMessage(m_topology, target, type, msg, signature);

	// outgoing message format: target (string), type (int32_t), msg (string), signature (string)
	m_chanAtCrypto.send(boost::lexical_cast<string, int32_t>(CMT_SIGN_RESPONSE), true);
	m_chanAtCrypto.send(target, true);
	m_chanAtCrypto.send(typeStr, true);
	m_chanAtCrypto.send(msg, true);
	m_chanAtCrypto.send(signature);
}

void CryptoThread::verifyMessage()
{
	assert(m_signer != NULL);

	// incoming message format: from (string), type (int32_t), msg (string), signature (string)
	string from, typeStr, msg, signature;
	int32_t type;

	m_chanAtCrypto.recv(from);
	m_chanAtCrypto.recv(typeStr);
	type = boost::lexical_cast<int32_t>(typeStr);
	m_chanAtCrypto.recv(msg);
	m_chanAtCrypto.recv(signature);

	bool correct = m_signer->verifyMessage(m_topology, from, type, msg, signature);

	// outgoing message format: from (string), type (int32_t), msg (string), valid (bool)
	m_chanAtCrypto.send(boost::lexical_cast<string, int32_t>(CMT_VERIFY_RESPONSE), true);
	m_chanAtCrypto.send(from, true);
	m_chanAtCrypto.send(typeStr, true);
	m_chanAtCrypto.send(msg, true);
	m_chanAtCrypto.send(boost::lexical_cast<string, bool>(correct));
}

void CryptoThread::requestSign(const std::string& target, const int32_t type, const std::string& msg)
{
	LOG_DEBUG("Node has requested signing a message");

	// outgoing message format: target (string), type (int32_t), msg (string)
	m_chanAtNode.send(boost::lexical_cast<string, int32_t>(CMT_SIGN_REQUEST), true);
	m_chanAtNode.send(target, true);
	m_chanAtNode.send(boost::lexical_cast<string>(type), true);
	m_chanAtNode.send(msg);
}

void CryptoThread::requestVerify(const std::string& from, const int32_t type, const std::string& msg, const std::string& signature)
{
	LOG_DEBUG("Node has requested verifying a signature");

	// incoming message format: from (string), type (int32_t), msg (string), signature (string)
	m_chanAtNode.send(boost::lexical_cast<string, int32_t>(CMT_VERIFY_REQUEST), true);
	m_chanAtNode.send(from, true);
	m_chanAtNode.send(boost::lexical_cast<string>(type), true);
	m_chanAtNode.send(msg, true);
	m_chanAtNode.send(signature);
}

} /* namespace bento */
