/*
 * topology.h
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#ifndef TOPOLOGY_H_
#define TOPOLOGY_H_

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

namespace bento {

class Topology {
public:
	typedef std::vector<std::string> NodeList;
    typedef boost::unordered_map<std::string, std::string> AddressList;
    typedef boost::unordered_map<std::string, AddressList> TopologyMap;

	Topology(const std::string& ownerName);
	virtual ~Topology();

    inline const std::string& getOwnerName() { return m_ownerName; }

    inline const NodeList& getNodeList() { return m_nodeList; }
    inline const NodeList& getNeighbours() { return m_neighbours; }
    const AddressList& getNeighbourAddresses();

    void updateTopologyMap(const NodeList& nodeList, const TopologyMap& topologyMap);

    void debugPrint();
private:
    std::string m_ownerName;

    NodeList m_nodeList;
    NodeList m_neighbours;

    TopologyMap m_topology;
};

} /* namespace bento */

#endif /* TOPOLOGY_H_ */
