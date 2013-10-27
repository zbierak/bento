/*
 * zmq-helpers.h
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#ifndef ZMQ_HELPERS_H_
#define ZMQ_HELPERS_H_

#include <zmq.hpp>

namespace bento
{
	bool zmqSend(zmq::socket_t* sock, const std::string& msg, bool more = false);
	bool zmqSend(zmq::socket_t* sock, bool more = false);

	bool zmqSignalSend(zmq::socket_t* sock, const std::string& signal, bool more = false);

	bool zmqRecv(zmq::socket_t* sock, std::string& result);
	bool zmqRecv(zmq::socket_t* sock);

	bool zmqBind(zmq::socket_t* sock, unsigned port, const std::string& proto = "tcp");
	bool zmqConnect(zmq::socket_t* sock, const std::string& addr, unsigned port, const std::string& proto = "tcp");
	bool zmqConnect(zmq::socket_t* sock, const std::string& addr, const std::string& proto = "tcp");
}

#endif /* ZMQ_HELPERS_H_ */
