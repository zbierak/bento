/*
 * sender.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: zbierak
 */

#include "sender.h"

#include "../zmq-wrappers/zmq-helpers.h"
#include "../zmq-wrappers/zmq-context.h"

#include "../exceptions.h"
#include "../logger.h"
#include "../signals.h"

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

namespace bento {

const std::string SENDER_INIT_CHANNEL_NAME = "SENDER_INIT_CHANNEL_FOR_NODE_";
SenderInitChannelAtSender::SenderInitChannelAtSender(const std::string nodeName): InprocChannelSlave(SENDER_INIT_CHANNEL_NAME+nodeName) {}
SenderInitChannelAtNode::SenderInitChannelAtNode(const std::string nodeName): InprocChannelMaster(SENDER_INIT_CHANNEL_NAME+nodeName) {}

Sender::Sender(Topology* topology): m_topology(topology), m_initNotify(topology->getOwnerName())
{
	m_thread = new boost::thread(boost::bind(&Sender::run, this));
}

Sender::~Sender()
{
    for (SocketMap::iterator it = m_socketMap.begin(); it != m_socketMap.end(); ++it)
        delete it->second;
    m_socketMap.clear();

    m_thread->join();
    delete m_thread;
}

void Sender::run()
{
    // TODO: this should be moved to a settings file
    const int MAX_RESPONSE_WAIT = 1000;
    const int MAX_RETRY_COUNT = -1;

    Topology::AddressList notConnected = Topology::AddressList(m_topology->getNeighbourAddresses());

    int retries = 0;
    while (!notConnected.empty())
    {
    	if (MAX_RETRY_COUNT >= 0 && retries > MAX_RETRY_COUNT)
    		throw GeneralException("Unable to connect all neighbours (used all "+boost::lexical_cast<string>(MAX_RETRY_COUNT)+" attempt(s)");

    	for (Topology::AddressList::const_iterator it=notConnected.begin(); it!=notConnected.end(); )
    	{
    		zmq::socket_t* sock = new zmq::socket_t(Context::getInstance(), ZMQ_DEALER);
    		if (zmqConnect(sock, it->second))
    		{
    			zmqSignalSend(sock, SIGNAL_HELLO, true);
    			zmqSend(sock, m_topology->getOwnerName(), false);

    			string msg;
    			bool success = zmqRecv(sock, msg, MAX_RESPONSE_WAIT);

    			if (success)
    			{
    				if (!msg.empty())
    				{
    					LOG_ERROR("Was expecting signal but got a normal message");
    					success = false;
    				}
    				else
    				{
    					zmqRecv(sock, msg);
    					if (msg != SIGNAL_HELLO_OK)
    					{
    						LOG_ERROR("Was expecting HELLO_OK signal but got %s", msg.c_str());
    						success = false;
    					}
    				}
    			}

    			if (success)
    			{
    				m_socketMap[it->first] = sock;
    				it = notConnected.erase(it);
    			}
    			else
    			{
    				delete sock;
    				LOG_ERROR("Unable to connect to: %s", it->first.c_str());
    				++it;
    			}
    		}
    		else
    		{
    			delete sock;
    		}
    	}

    	retries++;
    }

    LOG_DEBUG("The sender has been initialized.");
    m_initNotify.send();

    // end of thread
    // TODO: notify Node* using callback function (onSenderReady) and pass pointer to self
}

bool Sender::send(const std::string& target, const int32_t type, const std::string& msg)
{
    SocketMap::iterator it = m_socketMap.find(target);
    if (it == m_socketMap.end())
        throw GeneralException("Unable to find target "+target+" in provided topology.");

    string sType = boost::lexical_cast<string>(type);

    bool result = true;
    result = zmqSend(it->second, sType, true) && result;
    result = zmqSend(it->second, msg, false) && result;

    return result;
}

zmq::socket_t* Sender::getSocket(const std::string& target)
{
	SocketMap::iterator it = m_socketMap.find(target);
	if (it == m_socketMap.end())
		throw GeneralException("Unable to find target "+target+" in provided topology.");
	return it->second;
}

} /* namespace bento */


