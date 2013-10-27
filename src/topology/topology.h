/*
 * topology.h
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#ifndef TOPOLOGY_H_
#define TOPOLOGY_H_

#include <zmq.hpp>
#include <boost/unordered_map.hpp>

namespace bento {

class Topology {
public:
	Topology(const std::string& ownerName);
	virtual ~Topology();

	// association on incoming connections
	void addIncoming(const std::string& zmqId, const std::string& name);
	bool getIncomingName(const std::string& zmqId, std::string& name);
private:
	typedef boost::unordered_map<std::string, zmq::socket_t*> SocketMap;
	SocketMap m_socketMap;
};

} /* namespace bento */

#endif /* TOPOLOGY_H_ */
