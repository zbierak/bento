/*
 * sender.h
 *
 *  Created on: Oct 28, 2013
 *      Author: zbierak
 */

#ifndef SENDER_H_
#define SENDER_H_

#include <zmq.hpp>

#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>

#include "../zmq-wrappers/zmq-inproc.h"

#include "topology.h"

namespace bento {

/*
 * Channels for notifying the Node that the sender has been initialized
 */
class SenderInitChannelAtSender: public InprocChannelSlave
{
public:
	SenderInitChannelAtSender(const std::string nodeName);
};

class SenderInitChannelAtNode: public InprocChannelMaster
{
public:
	SenderInitChannelAtNode(const std::string nodeName);
};

/*
 * Class for sending messages to Node's neighbors, according to selected topology
 */
class Sender
{
public:
    Sender(Topology* topology);
    virtual ~Sender();

    bool send(const std::string& target, const int32_t type, const std::string& msg, const std::string& signature = "");

    zmq::socket_t* getSocket(const std::string& target);
private:
    void run();

    typedef boost::unordered_map<std::string, zmq::socket_t*> SocketMap;
    SocketMap m_socketMap;

    Topology* m_topology;

    boost::thread* m_thread;

    SenderInitChannelAtSender m_initNotify;

    bool m_initAbort;
};

} /* namespace bento */

#endif /* SENDER_H_ */
