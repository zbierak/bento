/*
 * node.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#include "node.h"
#include "zmq-context.h"
#include "zmq-helpers.h"

#include <iostream>

using namespace std;

namespace bento {

Node::Node(const std::string& name, unsigned port): m_incomingSock(Context::getInstance(), ZMQ_ROUTER), m_running(false)
{
	zmqBind(&m_incomingSock, port);
}

Node::~Node()
{
}

void Node::run()
{
	m_running = true;

	zmq::pollitem_t items [] = {
			{ m_incomingSock, 0, ZMQ_POLLIN, 0 }
	};

	while (m_running)
	{
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

					//addIncoming(const std::string& zmqId, const std::string& name);
				}
				else
				{
					cerr << "Unrecognized signal: " << msg << endl;
				}
			}
		}
	}

}

} /* namespace bento */
