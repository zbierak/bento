/*
 * topology-status.cpp
 *
 *  Created on: Mar 26, 2014
 *      Author: zbierak
 */

#include "topology-status.h"

#include "../logger.h"
#include "../timers/timer.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace bento {

const std::string TopologyStatus::TS_DELIMITER = ",";

TopologyStatus::TopologyStatus(Node* node, int32_t msgType):
		m_node(node),
		m_msgType(msgType),
		m_inRequest(false),
		m_timeoutId(-1)
{
	m_myNextRequestId = Timer::Now::milliseconds();
}

TopologyStatus::~TopologyStatus() { }

bool TopologyStatus::onMessage(const std::string& from, const int32_t type, const std::string& msg)
{
	if (type != m_msgType)
		return false;

	std::vector<std::string> params;
	boost::split(params, msg, boost::is_any_of(TS_DELIMITER));

	if (params.size() < 3)
	{
		LOG_WARN("Improper message format in TopologyStatus service, dropping the message.");
		return false;
	}

	string starter = params[0];
	uint64_t requestId = boost::lexical_cast<uint64_t>(params[1]);

	NotifiedNodesSet& nns = m_history[starter];

	// get the last request version
	uint64_t lastKnownRequest = 0;
	LastRequestMap::const_iterator lastIt = m_lastRequest.find(starter);
	if (lastIt != m_lastRequest.end())
		lastKnownRequest = lastIt->second;

	// compare the version of the TS request within the node and the message
	if (lastKnownRequest > requestId)
	{
		// a message from an old request, discard it
		return true;
	}
	else if (lastKnownRequest < requestId)
	{
		// a brand new request, cleanup from the last one
		nns.clear();
	}

	size_t lastSize = nns.size();
	for (unsigned i=2; i<params.size(); i++)
	{
		nns.insert(params[i]);
	}

	nns.insert(m_node->getName());
	m_lastRequest[starter] = requestId;

	// pass this message only if new nodes have arrived since the last time I have passed this message
	if (nns.size() > lastSize)
	{
		string msg = params[0] + TS_DELIMITER + params[1];

		for (NotifiedNodesSet::const_iterator it = nns.begin(); it != nns.end(); ++it)
			msg += TS_DELIMITER + (*it);

		m_node->pass(m_msgType, msg);
	}

	if (m_inRequest && m_node->getName() == starter && nns.size() == m_node->getTopology().getNodeList().size())
	{
		m_node->cancelTimeout(m_timeoutId);
		m_inRequest = false;
		m_callback(m_node->getTopology().getNodeList(), Topology::NodeList());
	}

	return true;
}

bool TopologyStatus::query(StatusCallback callback, uint32_t timeout)
{
	if (m_inRequest)
		return false;

	m_inRequest = true;
	m_callback = callback;

	// we are starting a new ts request
	string starter = m_node->getName();
	m_lastRequest[starter] = m_myNextRequestId;
	NotifiedNodesSet& nns = m_history[starter];
	nns.insert(starter);

	// The message has a format: <starter_name>,<starter_request_id>,<notified1>,<notified2>,...
	string msg = starter + TS_DELIMITER + boost::lexical_cast<string>(m_myNextRequestId) + TS_DELIMITER + starter;
	m_node->pass(m_msgType, msg);

	m_timeoutId = m_node->setTimeout(timeout, boost::bind(&TopologyStatus::onQueryTimeout, this));

	m_myNextRequestId = Timer::Now::milliseconds();
	return true;
}

void TopologyStatus::onQueryTimeout()
{
	string starter = m_node->getName();
	NotifiedNodesSet& nns = m_history[starter];

	Topology::NodeList found, notFound;

	const Topology::NodeList& topologyNodes = m_node->getTopology().getNodeList();
	for (Topology::NodeList::const_iterator it = topologyNodes.begin(); it != topologyNodes.end(); ++it)
	{
		if (nns.find(*it) == nns.end())
			notFound.push_back(*it);
		else
			found.push_back(*it);
	}

	m_inRequest = false;
	m_callback(found, notFound);
}

} /* namespace bento */
