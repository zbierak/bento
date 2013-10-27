/*
 * node.h
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#ifndef NODE_H_
#define NODE_H_

#include <map>
#include <string>
#include <zmq.hpp>

namespace bento {

class Node
{
public:
	Node(const std::string& name, unsigned port);
	virtual ~Node();

	void run();
private:
	zmq::socket_t m_incomingSock;
	std::string m_name;

	bool m_running;
};

} /* namespace bento */

#endif /* NODE_H_ */
