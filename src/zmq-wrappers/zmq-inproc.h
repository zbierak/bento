/*
 * zmq-inproc.h
 *
 *  Created on: Nov 6, 2013
 *      Author: zbierak
 */

#ifndef ZMQ_INPROC_H_
#define ZMQ_INPROC_H_

#include <zmq.hpp>

namespace bento {

class InprocChannel
{
public:
	InprocChannel(const std::string& name);
	virtual ~InprocChannel();

	bool send(const std::string& msg, bool more = false);
	bool send(bool more = false);

	bool recv(std::string& result);
	bool recv();

	inline zmq::socket_t* getSocket() { return m_sock; }
protected:

	zmq::socket_t* m_sock;
};

class InprocChannelMaster: public InprocChannel
{
public:
	InprocChannelMaster(const std::string& name);
};

class InprocChannelSlave: public InprocChannel
{
public:
	InprocChannelSlave(const std::string& name);
};

} /* namespace bento */

#endif /* ZMQ_INPROC_H_ */
