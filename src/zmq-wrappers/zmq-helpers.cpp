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

	bool zmqSignalSend(zmq::socket_t* sock, const std::string& signal, bool more)
	{
		return zmqSend(sock, true) &&
			zmqSend(sock, signal, more);
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

    bool zmqRecv(zmq::socket_t* sock, std::string& result, int timeout)
    {
        zmq::pollitem_t items[] = {
                { *sock, 0, ZMQ_POLLIN, 0 }
        };
        zmq::poll(&items[0], 1, timeout);

        if (items[0].revents & ZMQ_POLLIN)
            return zmqRecv(sock, result);
        else
            return false;
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
		return zmqConnect(sock, addr + ":" + boost::lexical_cast<string>(port), proto);
	}

	bool zmqConnect(zmq::socket_t* sock, const std::string& addr, const std::string& proto)
	{
		string endpoint = proto + "://" + addr;

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
