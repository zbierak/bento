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

namespace bento {

class Node
{
public:
	Node();
	virtual ~Node();

	// initialize bento messaging system. This function needs to be run by EXACTLY one node
	void initBento(const std::string& myId, const std::string& myAddr, const std::map<std::string, std::string>& secondaryAddr);
};

} /* namespace bento */

#endif /* NODE_H_ */
