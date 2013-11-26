/*
 * node.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#include "node.h"
#include "logger.h"
#include "signals.h"
#include "exceptions.h"

#include "zmq-wrappers/zmq-context.h"
#include "zmq-wrappers/zmq-helpers.h"

#include "topology/sender.h"

#include <boost/lexical_cast.hpp>

#include <iostream>

using namespace std;

namespace bento {

const std::string NODE_INFO_CHANNEL_PREFIX = "INFO_CHANNEL_NODE_";

const std::string INITIALIZE_TOPOLOGY_MSG = "initialize-topology";
const std::string TERMINATE_NODE_MSG = "terminate-node";

Node::Node(const Topology& topology):
	m_topology(topology),
	m_name(topology.getOwnerName()),
    m_incomingSock(Context::getInstance(), ZMQ_ROUTER),
    m_running(false),
    m_sender(NULL),
    m_senderUnderInit(NULL),
    m_thread(NULL),
    m_senderReady(false),
    m_infoChannelMaster(NODE_INFO_CHANNEL_PREFIX+topology.getOwnerName()),
    m_messageSigner(NULL)
{

	zmqBind(&m_incomingSock, m_topology.getOwnerPort());
}

Node::~Node()
{
	if (m_sender != NULL)
		delete m_sender;
	if (m_senderUnderInit != NULL)
		delete m_senderUnderInit;
}

void Node::start()
{
	m_thread = new boost::thread(boost::bind(&Node::run, this));
}

void Node::stop()
{
	m_infoChannelMaster.send(TERMINATE_NODE_MSG);

	if (m_thread != NULL)
	{
		m_thread->join();
		delete m_thread;
		m_thread = NULL;
	}
}

const std::string& Node::getName()
{
	return m_name;
}

void Node::connectTopology()
{
	// topology can be initialized only once per node
	if (m_sender != NULL || m_senderUnderInit != NULL)
		return;

	m_infoChannelMaster.send(INITIALIZE_TOPOLOGY_MSG);
}

void Node::run()
{
	m_running = true;

	SenderInitChannelAtNode notifyChannel(m_name);
	InprocChannelSlave infoChannelSlave(NODE_INFO_CHANNEL_PREFIX+m_name);

	const unsigned POLL_ITEMS_SIZE = 3;
	zmq::pollitem_t items[POLL_ITEMS_SIZE] = {
			{ m_incomingSock, 0, ZMQ_POLLIN, 0 },
			{ *notifyChannel.getSocket(), 0, ZMQ_POLLIN, 0 },
			{ *infoChannelSlave.getSocket(), 0, ZMQ_POLLIN, 0 }
	};


	LOG_DEBUG("Node entering main loop");

	int timeToNextEvent = -1;

	while (m_running)
	{
        zmq::poll(&items[0], POLL_ITEMS_SIZE, timeToNextEvent);

        LOG_DEBUG("Node has new messages");

        // m_incomingSock
		if (items[0].revents & ZMQ_POLLIN)
		{
			// all messages exchanged through incomingSock are in format:
			//     [message or <empty> signal (optional data)]
			// However, as this is a router socket, we also need to receive
			// the sender id assigned by zmq.

			string sender, msg;

			zmqRecv(&m_incomingSock, sender);
			zmqRecv(&m_incomingSock, msg);

			if (!msg.empty())
			{
				// normal message contains of message type, message contents and a signature
				string from, signature;
				int32_t type = boost::lexical_cast<int32_t>(msg);
				zmqRecv(&m_incomingSock, msg);
				zmqRecv(&m_incomingSock, signature);

				if (!m_incomingRegistry.getName(sender, from))
				{
					LOG_ERROR("Message sender not found in incoming registry. Some serious conceptual error exists in the course of the library, as this would not have happened otherwise.");
					continue;
				}

				if (m_sender == NULL)
				{
					// not yet connected, we need to buffer the message
					m_unhandledMessages.push_back(boost::make_tuple(from, type, msg, signature));
				}
				else
				{
					// node is fully connected, we can handle it straightway
					processOnMessage(from, type, msg, signature);
				}
			}
			else
			{
				// it is a signal, receive it
				zmqRecv(&m_incomingSock, msg);

				if (msg == SIGNAL_HELLO)
				{
					// as a parameter to signal hello the sender name is passed
					string identity;
					zmqRecv(&m_incomingSock, identity);
                    m_incomingRegistry.addName(sender, identity);

                    // respond with HELLO_OK signal
                    zmqSend(&m_incomingSock, sender, true);
                    zmqSend(&m_incomingSock, true);
                    zmqSend(&m_incomingSock, SIGNAL_HELLO_OK, false);

                    checkIfHasConnected();
				}
				else if (msg == SIGNAL_INTRODUCE_YOURSELF)
				{
					string respondTo;
					zmqRecv(&m_incomingSock, respondTo);

					// only introduce yourself when you actually are connected
					// otherwise it means you are in the middle of connection
					// process and will, sooner or later, introduce yourself
					if (m_sender != NULL)
					{
						zmq::socket_t* sock = m_sender->getSocket(respondTo);
						zmqSignalSend(sock, SIGNAL_HELLO, true);
						zmqSend(sock, m_topology.getOwnerName(), false);

					}
				}
				else
				{
					cerr << "Unrecognized signal: " << msg << endl;
				}
			}
		}

		// notifyChannel - has connected to all its neighbors
		if (items[1].revents & ZMQ_POLLIN)
		{
			// remove the message so that it doesn't stall the channel
			notifyChannel.recv();

			// store info that has connected to all its neighbors
			m_senderReady = true;

			// check if also all neighbors connected, if so, notify the derived class
			checkIfHasConnected();
		}

		// info channel
		if (items[2].revents & ZMQ_POLLIN)
		{
			string msg;
			infoChannelSlave.recv(msg);

			LOG_DEBUG("MESSAGE: %s", msg.c_str());

			if (msg == INITIALIZE_TOPOLOGY_MSG)
			{
				LOG_DEBUG("Initializing topology");
				m_senderUnderInit = new Sender(&m_topology);
			}
			else if (msg == TERMINATE_NODE_MSG)
			{
				LOG_DEBUG("Terminating the application");
				m_running = false;
			}
			else
			{
				LOG_ERROR("Unexpected message %s", msg.c_str());
			}

		}

		// process timers (if any) and estimate time to the next one
		timeToNextEvent = m_timerManager.processReady();

	}

	LOG_DEBUG("Node %s is preparing to terminate", m_topology.getOwnerName().c_str());
}

void Node::checkIfHasConnected()
{
	// do not check if ready to connect if already connected
	if (m_sender != NULL)
		return;

	/*
	 * in order to notify the derived class, two conditions must be met:
	 * 1) node has connected to all its neighbors
	 * 2) all nodes that have this node as neighbors must be connected to
	 *    this machine.
	 */

	// condition one
	if (!m_senderReady)
		return;

	// condition two
	bool everybodyHasConnectedToMe = true;
	const Topology::NodeList& amNeighbourOf = m_topology.whoseNeighborAmI();
	for (Topology::NodeList::const_iterator it = amNeighbourOf.begin(); it != amNeighbourOf.end(); ++it)
	{
		if (!m_incomingRegistry.containsName(*it))
		{
			everybodyHasConnectedToMe = false;

			/*
			 * This node is not yet connected to us. Normally, we would just wait
			 * for it to connect (as we would not receive any messages from it
			 * until it would introduce itself). However, when our node exists and
			 * attempts to reconnect to an existing topology, the other node would
			 * not introduce itself, as it is unaware that we have lost our connection.
			 * So, we need to send it a signal to introduce itself. We only do it once
			 * for every machine that is our neighbor though.
			 */
			if (m_introduceRequested.find(*it) == m_introduceRequested.end())
			{
				zmq::socket_t* theSocket = m_senderUnderInit->getSocket(*it);
				zmqSignalSend(theSocket, SIGNAL_INTRODUCE_YOURSELF, true);
				zmqSend(theSocket, m_topology.getOwnerName(), false);
				m_introduceRequested.insert(*it);
			}
		}
	}

	if (!everybodyHasConnectedToMe)
		return;

	LOG_DEBUG("All channels to and from the node are connected.");

	// conditions have been met, make the sender usable to the Node
	m_sender = m_senderUnderInit;
	m_senderUnderInit = NULL;

	// notify that the node is ready
	this->onConnect();

	if (!m_unhandledMessages.empty())
	{
		// handle buffered messages
		for (MessageBuffer::const_iterator it = m_unhandledMessages.begin(); it != m_unhandledMessages.end(); ++it)
		{
			processOnMessage(it->get<0>(), it->get<1>(), it->get<2>(), it->get<3>());
		}

		m_unhandledMessages.clear();
	}
}

bool Node::send(const std::string& target, const std::string& msg)
{
	return this->send(target, -1, msg);
}

bool Node::send(const std::string& target, const int32_t type, const std::string& msg)
{
	if (m_sender == NULL)
	{
		LOG_INFO("Sender is not connected yet, cannot send the message");
		return false;
	}

	string signature;
	if (m_messageSigner != NULL)
	{
		m_messageSigner->signMessage(this->getTopology(), target, type, msg, signature);
	}

	return m_sender->send(target, type, msg, signature);
}

bool Node::pass(const std::string& msg)
{
	return this->pass(-1, msg);
}

bool Node::pass(const int32_t type, const std::string& msg)
{
	bool success = true;
	const Topology::NodeList& neighbours = m_topology.getNeighbours();
	for (Topology::NodeList::const_iterator it = neighbours.begin(); it != neighbours.end(); ++it)
	{
		success = this->send(*it, type, msg);
		if (!success)
			break;
	}

	return success;
}

bool Node::passInGroup(const std::string& msg)
{
	return this->passInGroup(-1, msg);
}

bool Node::passInGroup(const int32_t type, const std::string& msg)
{
	bool success = true;
	const Topology::NodeList& neighbours = m_topology.getSameGroupNeighbours();
	for (Topology::NodeList::const_iterator it = neighbours.begin(); it != neighbours.end(); ++it)
	{
		success = this->send(*it, type, msg);
		if (!success)
			break;
	}

	return success;
}

void Node::registerGatherMessage(const int32_t type, unsigned minMessages)
{
	m_gatherRegistry.registerMessageType(type, minMessages);
}

int Node::setTimeout(unsigned timeout, const TimerEvent::TimeoutCallback& callback)
{
	return m_timerManager.setTimeout(timeout, callback);
}

bool Node::cancelTimeout(int timeoutId)
{
	return m_timerManager.cancelTimeout(timeoutId);
}

void Node::processOnMessage(const std::string& from, const int32_t type, const std::string& msg, const std::string& signature)
{
	if (m_messageSigner != NULL)
	{
		if (!m_messageSigner->verifyMessage(this->getTopology(), from, type, msg, signature))
		{
			LOG_INFO("Obtained message from %s of type %d that failed signature verification.", from.c_str(), type);
			return;
		}
	}

	// pass onMessage event only when GatherRegistry permits (note that by default it permits when
	// a message type is not registered)
	if (m_gatherRegistry.onMessage(from, type, msg))
		this->onMessage(from, type, msg);
}

} /* namespace bento */
