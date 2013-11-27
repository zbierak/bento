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
#include <boost/unordered_set.hpp>

#include "topology/topology.h"
#include "topology/incoming-registry.h"
#include "topology/sender.h"

#include "gather/gather-registry.h"
#include "timers/timer-manager.h"
#include "zmq-wrappers/zmq-inproc.h"

#include "message-signer.h"

namespace bento {

class Node
{
public:
	Node(const Topology& topology);
	virtual ~Node();

	// run in the same thread
	void run();

	// start as a separate thread
	void start();

	// stop node (can be called from (almost) any thread). However, it never can be called
	// from the Node thread (i.e. from onConnect, onMessage etc), or it will hang.
	void stop();

	void connectTopology();

	inline void setMessageSigner(IMessageSigner* messageSigner) { m_messageSigner = messageSigner; }
protected:
    virtual void onConnect() = 0;
    virtual void onMessage(const std::string& from, const int32_t type, const std::string& msg) = 0;

    bool send(const std::string& target, const std::string& msg);
    bool send(const std::string& target, const int32_t type, const std::string& msg);

    bool pass(const std::string& msg);
    bool pass(const int32_t type, const std::string& msg);

    bool passInGroup(const std::string& msg);
    bool passInGroup(const int32_t type, const std::string& msg);

    bool passInGroup(const std::string& groupName, const std::string& msg);
    bool passInGroup(const std::string& groupName, const int32_t type, const std::string& msg);

    int setTimeout(unsigned timeout, const TimerEvent::TimeoutCallback& callback);
    bool cancelTimeout(int timeoutId);

    const std::string& getName();
    inline const Topology& getTopology() const { return m_topology; }

    void registerGatherMessage(const int32_t type, unsigned minMessages);
private:
    Sender* m_sender;
    Sender* m_senderUnderInit;

    GatherRegistry m_gatherRegistry;

	zmq::socket_t m_incomingSock;
    IncomingRegistry m_incomingRegistry;

    Topology m_topology;
	std::string m_name;

	TimerManager m_timerManager;

	bool m_running;

	boost::thread* m_thread;
	InprocChannelMaster m_infoChannelMaster;

	IMessageSigner* m_messageSigner;

	typedef std::vector<boost::tuple<std::string, int32_t, std::string, std::string> > MessageBuffer;
	MessageBuffer m_unhandledMessages;

	void processOnMessage(const std::string& from, const int32_t type, const std::string& msg, const std::string& signature);

	bool m_senderReady;
	boost::unordered_set<std::string> m_introduceRequested;
	void checkIfHasConnected();
};

} /* namespace bento */

#endif /* NODE_H_ */
