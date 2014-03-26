/*
 * topology-status.h
 *
 *  Created on: Mar 26, 2014
 *      Author: zbierak
 */

#ifndef TOPOLOGY_STATUS_H_
#define TOPOLOGY_STATUS_H_

#include "../node.h"
#include "../message-intercepter.h"

#include "../topology/topology.h"

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/function.hpp>

#include <vector>

namespace bento {

class Node;

/*
 * This service allows to check the current state of the topology by performing a broadcast
 * from this node and awaiting for a response from other nodes. Note that since bento does not
 * require for all paths to be bidirectional, this service might determine that not all nodes
 * are connected, even though they might indeed be. However, when all paths are bidirectional,
 * the result will be accurate.
 *
 * Since the TS service works above the node interface, you need to explicitly assign it with
 * a message type from your protocol that will be used for its communication.
 *
 * Btw, in order to work, this service needs to be added as a message intercepter on all nodes
 * you want to be included in the search.
 */
class TopologyStatus: public IMessageIntercepter {
public:
	typedef boost::function<void(const Topology::NodeList&, const Topology::NodeList&)> StatusCallback;

	TopologyStatus(Node* node, int32_t msgType);
	virtual ~TopologyStatus();

	bool onMessage(const std::string& from, const int32_t type, const std::string& msg);

	bool query(StatusCallback callback, uint32_t timeout);
private:
	void onQueryTimeout();

	static const std::string TS_DELIMITER;

	int32_t m_msgType;

	Node* m_node;

	uint64_t m_myNextRequestId;
	bool m_inRequest;
	StatusCallback m_callback;
	int m_timeoutId;

	// the id of last request issued by node <string>
	typedef boost::unordered_map<std::string, uint64_t> LastRequestMap;
	LastRequestMap m_lastRequest;

	typedef boost::unordered_set<std::string> NotifiedNodesSet;
	typedef boost::unordered_map<std::string, NotifiedNodesSet> RequestHistory;
	RequestHistory m_history;
};

typedef boost::shared_ptr<TopologyStatus> TopologyStatusPtr;

} /* namespace bento */

#endif /* TOPOLOGY_STATUS_H_ */
