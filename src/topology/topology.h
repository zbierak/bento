/*
 * topology.h
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#ifndef TOPOLOGY_H_
#define TOPOLOGY_H_

#include <string>

namespace bento {

class Topology {
public:
	Topology(const std::string& ownerName);
	virtual ~Topology();

    inline const std::string& getOwnerName() { return m_ownerName; }
private:
    std::string m_ownerName;
};

} /* namespace bento */

#endif /* TOPOLOGY_H_ */
