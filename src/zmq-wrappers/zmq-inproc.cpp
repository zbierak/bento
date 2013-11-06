/*
 * zmq-inproc.cpp
 *
 *  Created on: Nov 6, 2013
 *      Author: zbierak
 */

#include "zmq-inproc.h"
#include "zmq-context.h"
#include "zmq-helpers.h"

using namespace std;

namespace bento {

InprocChannel::InprocChannel(const std::string& name)
{
	m_sock = new zmq::socket_t(Context::getInstance(), ZMQ_PAIR);
}

InprocChannel::~InprocChannel()
{
	if (m_sock != NULL)
		delete m_sock;
}

bool InprocChannel::send(const std::string& msg, bool more)
{
	return zmqSend(m_sock, msg, more);
}

bool InprocChannel::send(bool more)
{
	return zmqSend(m_sock, more);
}

bool InprocChannel::recv(std::string& result)
{
	return zmqRecv(m_sock, result);
}

bool InprocChannel::recv()
{
	return zmqRecv(m_sock);
}

InprocChannelMaster::InprocChannelMaster(const std::string& name): InprocChannel(name)
{
	string addr = "inproc://"+name;
	m_sock->bind(addr.c_str());
}

InprocChannelSlave::InprocChannelSlave(const std::string& name): InprocChannel(name)
{
	string addr = "inproc://"+name;
	m_sock->connect(addr.c_str());
}



} /* namespace bento */
