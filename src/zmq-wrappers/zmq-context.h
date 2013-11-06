/*
 * zmq-context.h
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#ifndef ZMQ_CONTEXT_H_
#define ZMQ_CONTEXT_H_

#include <zmq.hpp>

namespace bento {

class Context
{
public:
	inline static zmq::context_t& getInstance()
	{
		static Context instance;
		return instance.m_context;
	}

private:
	Context(): m_context(1) {}
	Context(const Context&);
	Context& operator=(const Context&);
	~Context() {}

	zmq::context_t m_context;
};

} /* namespace bento */

#endif /* ZMQ_CONTEXT_H_ */
