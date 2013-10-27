/*
 * topology.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#include "topology.h"

#include "../node.h"
#include "../signals.h"
#include "../zmq-context.h"
#include "../zmq-helpers.h"

using namespace std;

namespace bento {

Topology::Topology(const std::string& ownerName)
{
	// TODO: these values should be read from file. I dream of a nice boost::spirit parser, perhaps in time...
	// For now we hardcode them here. Remember: it is only a temporary solution until I find some time for
	// writing the parser. Please, do not shoot ;)

	const unsigned NODE_COUNT = 2;

	string names[NODE_COUNT] = {"node1", "node2"};
	string addr[NODE_COUNT] = {"localhost:2013", "localhost:2014"};

	for (unsigned i=0; i<NODE_COUNT; i++)
	{
		if (names[i] != ownerName)
		{
			zmq::socket_t* sock = new zmq::socket_t(Context::getInstance());
			if (zmqConnect(sock, addr[i]))
			{
				m_socketMap.insert(make_pair(names[i], sock));
				zmqSignalSend(sock, SIGNAL_HELLO, true);
				zmqSend(sock, names[i], false);
			}
			else
			{
				delete sock;
			}
		}
	}
}

Topology::~Topology()
{
	for (SocketMap::iterator it = m_socketMap.begin(); it != m_socketMap.end(); ++it)
		delete it->second;
	m_socketMap.clear();
}

} /* namespace bento */
