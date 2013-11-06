/*
 * node.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#include "node.h"
#include "logger.h"
#include "signals.h"

#include "zmq-wrappers/zmq-context.h"
#include "zmq-wrappers/zmq-helpers.h"

#include "topology/sender.h"

#include <iostream>

using namespace std;

namespace bento {

const std::string NODE_INFO_CHANNEL_PREFIX = "INFO_CHANNEL_NODE_";

const std::string INITIALIZE_TOPOLOGY_MSG = "initialize-topology";
const std::string TERMINATE_NODE_MSG = "terminate-node";

Node::Node(const std::string& name, unsigned port):
	m_name(name),
    m_incomingSock(Context::getInstance(), ZMQ_ROUTER),
    m_running(false),
    m_topology(name),
    m_sender(NULL),
    m_senderUnderInit(NULL),
    m_thread(NULL),
    m_infoChannelMaster(NODE_INFO_CHANNEL_PREFIX+name)
{
	zmqBind(&m_incomingSock, port);
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

	while (m_running)
	{
        zmq::poll(&items[0], POLL_ITEMS_SIZE, -1);

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
				// normal message
				cerr << "Received normal message - this is not handled yet" << endl;

				// todo: please bear in mind that when a message is obtained, the node does not
				// have to be connected yet - therefore we need to buffer these messages until
				// it is connected
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
				}
				else
				{
					cerr << "Unrecognized signal: " << msg << endl;
				}
			}
		}

		// notifyChannel
		if (items[1].revents & ZMQ_POLLIN)
		{
			// remove the message so that it doesn't stall the channel
			notifyChannel.recv();

			// make the sender usable to the Node
			m_sender = m_senderUnderInit;
			m_senderUnderInit = NULL;

			// notify that the node is ready
			this->onConnect();
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
	}

	LOG_DEBUG("Node %s is preparing to terminate", m_topology.getOwnerName().c_str());
}

} /* namespace bento */