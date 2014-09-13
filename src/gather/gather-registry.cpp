/*
 * gather-registry.cpp
 *
 *  Created on: Nov 13, 2013
 *      Author: zbierak
 */

#include "gather-registry.h"

#include <assert.h>

#include <boost/lexical_cast.hpp>

#include "../logger.h"

#ifdef USE_HASH_FOR_GATHER
#include <cryptopp/sha.h>
#endif

using namespace std;

namespace bento {

GatherRegistry::GatherRegistry()
{

}

GatherRegistry::~GatherRegistry()
{

}

void GatherRegistry::registerMessageType(int32_t type, unsigned minMessages)
{
	m_requiredMinimum[type] = minMessages;
	m_awaitingMessages.insert(make_pair(type, MessageMap()));

	if (minMessages == 0)
	{
		LOG_WARN("Passed zero as the minMessages for message type %d. This message will never be delivered.", type);
	}

}

void GatherRegistry::deregisterMessageType(int32_t type)
{
	AmountMap::iterator rmIt = m_requiredMinimum.find(type);
	if (rmIt != m_requiredMinimum.end())
		m_requiredMinimum.erase(rmIt);

	TypesMap::iterator amIt = m_awaitingMessages.find(type);
	if (amIt != m_awaitingMessages.end())
		m_awaitingMessages.erase(amIt);
}

string buildMessageKey(int32_t type, const std::string& msg)
{
	string key = boost::lexical_cast<string>(type) + ":" + msg;

#ifdef USE_HASH_FOR_GATHER
	if (key.length() <= CryptoPP::SHA256::DIGESTSIZE)
		return key;

	unsigned char digest[CryptoPP::SHA256::DIGESTSIZE];
	CryptoPP::SHA256().CalculateDigest(digest, (const unsigned char*) key.c_str(), key.length());
	return string((char*)digest, CryptoPP::SHA256::DIGESTSIZE);
#else
	return key;
#endif
}

bool GatherRegistry::onMessage(const std::string& from, int32_t type, const std::string& msg)
{
	AmountMap::const_iterator minimumIt = m_requiredMinimum.find(type);
	if (minimumIt == m_requiredMinimum.end())
		return true;

	TypesMap::iterator typeIt = m_awaitingMessages.find(type);
	assert(typeIt != m_awaitingMessages.end());

	MessageMap& awaiting = typeIt->second;
	MessageMap::iterator messageIt = awaiting.find(msg);

	string messageKey;

	if (messageIt == awaiting.end())
	{
		messageKey = buildMessageKey(type, msg);

		// either such message has not been obtained or it has already been processed
		if (m_processedMessages.find(messageKey) != m_processedMessages.end())
		{
			// it has already been processed
			return false;
		}

		awaiting.insert(make_pair(msg, boost::shared_ptr<GatherMessageInfo>(new GatherMessageInfo())));
		messageIt = awaiting.find(msg);
		assert(messageIt != awaiting.end());
	}

	unsigned received = messageIt->second->addSender(from);

	if (received == minimumIt->second)
	{
		// this is exactly the number of responses we wanted to obtain. Move message to processed.
		if (messageKey.empty())
			messageKey = buildMessageKey(type, msg);

		m_processedMessages.insert(messageKey);
		awaiting.erase(messageIt);
		return true;
	}

	return false;
}

void GatherRegistry::cleanup()
{
	// erase processed messages (warning: this might lead to a situation when a message is delivered for a second time)
	m_processedMessages.clear();
}

} /* namespace bento */
