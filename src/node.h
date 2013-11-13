/*
 * node.h
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#ifndef NODE_H_
#define NODE_H_

#include <map>
#include <string>
#include <zmq.hpp>

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>

#include "topology/topology.h"
#include "topology/incoming-registry.h"
#include "topology/sender.h"

#include "zmq-wrappers/zmq-inproc.h"

namespace bento {

class Node
{
public:
	Node(const std::string& name, unsigned port);
	virtual ~Node();

	// run in the same thread
	void run();

	// start as a separate thread
	void start();

	// stop node (can be called from (almost) any thread). However, it never can be called
	// from the Node thread (i.e. from onConnect, onMessage etc), or it will hang.
	void stop();

	void connectTopology();
protected:
    virtual void onConnect() = 0;
    virtual void onMessage(const std::string& from, const int32_t type, const std::string& msg) = 0;

    bool send(const std::string& target, const std::string& msg);
    bool send(const std::string& target, const int32_t type, const std::string& msg);

    bool pass(const std::string& msg);
    bool pass(const int32_t type, const std::string& msg);

    const std::string& getName();
private:
    Sender* m_sender;
    Sender* m_senderUnderInit;

	zmq::socket_t m_incomingSock;
    IncomingRegistry m_incomingRegistry;

    Topology m_topology;

	std::string m_name;

	bool m_running;

	boost::thread* m_thread;
	InprocChannelMaster m_infoChannelMaster;

	typedef std::vector<boost::tuple<std::string, int32_t, std::string> > MessageBuffer;
	MessageBuffer m_unhandledMessages;
};

} /* namespace bento */

#endif /* NODE_H_ */
