/*
 * node.h
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#ifndef NODE_H_
#define NODE_H_

#include <map>
#include <vector>
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
#include "message-intercepter.h"

#include "crypto-thread.h"

#include "services/topology-status.h"

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

	// shutdown the node.
	void shutdown();

	void setMessageSigner(IMessageSigner* messageSigner);
	void setMessageSigners(const std::vector<IMessageSigner*>& messageSigners);
	bool getMessageSigners(std::vector<IMessageSigner*>& signers);

	void addMessageIntercepter(IMessageIntercepterPtr interceptor);

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

    inline const std::string& getName() const { return m_name; }
    inline const Topology& getTopology() const { return m_topology; }

    void registerGatherMessage(const int32_t type, unsigned minMessages);
    void deregisterGatherMessage(const int32_t type);

    /*
     * Make sure you know all implications when calling this function. Consult gather-registry.h beforehand.
     */
    void cleanupGatherRegistry();
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

	bool m_hasSigner;
	CryptoThread* m_cryptoThread;

	typedef std::vector<IMessageIntercepterPtr > InterceptersVector;
	InterceptersVector m_intercepters;

	typedef std::vector<boost::tuple<std::string, int32_t, std::string, std::string> > MessageBuffer;
	MessageBuffer m_unhandledMessages;

	boost::thread::id m_nodeThreadId;

	void processOnMessage(const std::string& from, const int32_t type, const std::string& msg, const std::string& signature);

	void onCryptoSign(const std::string& target, const int32_t type, const std::string& msg, const std::string& signature);
	void onCryptoVerify(const std::string& from, const int32_t type, const std::string& msg, bool success);

	bool m_senderReady;
	boost::unordered_set<std::string> m_introduceRequested;
	void checkIfHasConnected();

	friend class TopologyStatus;
};

} /* namespace bento */

#endif /* NODE_H_ */
