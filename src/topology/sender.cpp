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

    // TODO: these values should be read from file. I dream of a nice boost::spirit parser, perhaps in time...
    // For now we hardcode them here. Remember: it is only a temporary solution until I find some time for
    // writing the parser. Please, do not shoot ;)

    // btw, this code should be moved directly to topology, todo later on

    const unsigned NODE_COUNT = 2;

    string names[NODE_COUNT] = {"node1", "node2"};
    string addr[NODE_COUNT] = {"localhost:2013", "localhost:2014"};

    for (unsigned i=0; i<NODE_COUNT; i++)
    {
        if (names[i] != m_topology->getOwnerName())
        {
            zmq::socket_t* sock = new zmq::socket_t(Context::getInstance(), ZMQ_DEALER);
            if (zmqConnect(sock, addr[i]))
            {
                m_socketMap.insert(make_pair(names[i], sock));
                zmqSignalSend(sock, SIGNAL_HELLO, true);
                zmqSend(sock, names[i], false);

                string msg;
                bool success = zmqRecv(sock, msg, MAX_RESPONSE_WAIT);

                if (success)
                {
                    if (!msg.empty())
                    {
                        cerr << "Was expecting signal but got a normal message" << endl;
                        success = false;
                    }
                    else
                    {
                        zmqRecv(sock, msg);
                        if (msg != SIGNAL_HELLO_OK)
                        {
                            cerr << "Was expecting HELLO_OK signal but got " << msg << endl;
                            success = false;
                        }
                    }
                }

                if (!success)
                    throw GeneralException("Unable to connect to "+names[i]);

            }
            else
            {
                delete sock;
            }
        }
    }

    LOG_DEBUG("The sender has been initialized.");
    m_initNotify.send();

    // end of thread
    // TODO: notify Node* using callback function (onSenderReady) and pass pointer to self
}

bool Sender::send(const std::string& target, const std::string& msg)
{
    SocketMap::iterator it = m_socketMap.find(target);
    if (it == m_socketMap.end())
        throw GeneralException("Unable to find target "+target+" in provided topology.");

    return zmqSend(it->second, msg);
}

} /* namespace bento */
