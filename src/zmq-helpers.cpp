/*
 * zmq-helpers.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#include "zmq-helpers.h"

#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;

namespace bento
{
	bool zmqSend(zmq::socket_t* sock, const std::string& msg, bool more)
	{
		zmq::message_t m(msg.size());
		memcpy(m.data(), msg.data(), msg.size());
		return sock->send(m, more ? ZMQ_SNDMORE : 0);
	}

	bool zmqSend(zmq::socket_t* sock, bool more)
	{
		zmq::message_t m(0);
		return sock->send(m, more ? ZMQ_SNDMORE : 0);
	}

	bool zmqRecv(zmq::socket_t* sock, std::string& result)
	{
		zmq::message_t m;
		bool positive = sock->recv(&m);
		result = std::string(static_cast<char*>(m.data()), m.size());
		return positive;
	}

	bool zmqRecv(zmq::socket_t* sock)
	{
		zmq::message_t m;
		return sock->recv(&m);
	}

	bool zmqBind(zmq::socket_t* sock, unsigned port, const std::string& proto)
	{
		string endpoint = proto + "://*:" + boost::lexical_cast<string>(port);

		try
		{
			sock->bind(endpoint.c_str());
		}
		catch (const zmq::error_t& err)
		{
			cerr << err.what() << endl;
			return false;
		}

		return true;
	}

	bool zmqConnect(zmq::socket_t* sock, const std::string& addr, unsigned port, const std::string& proto)
	{
		string endpoint = proto + "://" + addr + ":" + boost::lexical_cast<string>(port);

		try
		{
			sock->connect(endpoint.c_str());
		}
		catch (const zmq::error_t& err)
		{
			cerr << err.what() << endl;
			return false;
		}

		return true;
	}
}
