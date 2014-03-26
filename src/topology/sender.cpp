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

Sender::Sender(Topology* topology): m_topology(topology), m_initNotify(topology->getOwnerName()), m_initAbort(false)
{
	m_thread = new boost::thread(boost::bind(&Sender::run, this));
}

Sender::~Sender()
{
    for (SocketMap::iterator it = m_socketMap.begin(); it != m_socketMap.end(); ++it)
        delete it->second;
    m_socketMap.clear();

    // close the thread asap and free the memory
    m_initAbort = true;
    m_thread->join();
    delete m_thread;
}

void Sender::run()
{
    // TODO: this should be moved to a settings file
    const int MAX_RESPONSE_WAIT = 1000;
    const int MAX_RETRY_COUNT = -1;

    // we discard the unsent messages straight away when the connection is closed
    // (they do not stall the channel and do not flood the newly connected machines
    //  when they take their time and not connect for a while)
    const int LINGER_TIME = 0;

    Topology::AddressList notConnected = Topology::AddressList(m_topology->getNeighbourAddresses());

    int retries = 0;
    while (!notConnected.empty())
    {
    	if (MAX_RETRY_COUNT >= 0 && retries > MAX_RETRY_COUNT)
    		throw GeneralException("Unable to connect all neighbours (used all "+boost::lexical_cast<string>(MAX_RETRY_COUNT)+" attempt(s)");

    	for (Topology::AddressList::const_iterator it=notConnected.begin(); it!=notConnected.end(); )
    	{
    		zmq::socket_t* sock = new zmq::socket_t(Context::getInstance(), ZMQ_DEALER);

    		sock->setsockopt(ZMQ_LINGER, &LINGER_TIME, sizeof(LINGER_TIME));
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
    				if (retries)
    				    LOG_INFO("Connected to: %s", it->first.c_str());

    				m_socketMap[it->first] = sock;
    				it = notConnected.erase(it);
    			}
    			else
    			{
    				if (!retries)
    				    LOG_INFO("Unable to connect to: %s", it->first.c_str());

    				sock->close();
    				delete sock;
    				++it;
    			}
    		}
    		else
    		{
    			delete sock;
    		}

    		if (m_initAbort)
    		{
    			// we are closing the application, do not notify, just quit
    			LOG_DEBUG("We are aborting the sender initiation.");
    			return;
    		}
    	}

    	retries++;
    }

    LOG_DEBUG("The sender has been initialized.");
    m_initNotify.send();

    // end of thread
}

bool Sender::send(const std::string& target, const int32_t type, const std::string& msg, const std::string& signature)
{
    SocketMap::iterator it = m_socketMap.find(target);
    if (it == m_socketMap.end())
        throw GeneralException("Unable to find target "+target+" in provided topology.");

    string sType = boost::lexical_cast<string>(type);

    bool result = true;
    result = zmqSend(it->second, sType, true) && result;
    result = zmqSend(it->second, msg, true) && result;
    result = zmqSend(it->second, signature, false) && result;

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

