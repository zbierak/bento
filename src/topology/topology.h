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
    typedef boost::unordered_map<std::string, std::string> RoleList;
    typedef boost::unordered_map<std::string, AddressList> TopologyMap;

	Topology(const std::string& ownerName, const std::string& topologyFileName);
	virtual ~Topology();

    inline const std::string& getOwnerName() const { return m_ownerName; }

    inline const NodeList& getNodeList() const { return m_nodeList; }
    inline const size_t getNodeCount() const { return m_nodeList.size(); }
    inline const NodeList& getNeighbours() const { return m_neighbours; }
    inline const NodeList& getSameGroupNeighbours() const { return m_sameGroupNeighbours; }
    const AddressList& getNeighbourAddresses() const;

    inline const std::string& getOwnerHost() const { return m_ownerHost; }
    inline const unsigned getOwnerPort() const { return m_ownerPort; }
    inline const NodeList& whoseNeighborAmI() const { return m_amNeighbourOf; }

    const std::string getOwnerRole() const;
    const std::string getRole(const std::string&) const;

    const unsigned getOwnerOrderNumber() const;
    const std::string getNodeWithOrderNumber(unsigned orderNumber) const;

    void updateTopologyMap(const NodeList& nodeList, const TopologyMap& topologyMap,
    		const AddressList& defaultAddresses, const RoleList& roles);

    void debugPrint() const;
private:
    std::string m_ownerName;

    std::string m_ownerHost;
    unsigned m_ownerPort;

    NodeList m_nodeList;
    NodeList m_neighbours;
    NodeList m_sameGroupNeighbours;
    NodeList m_amNeighbourOf;

    RoleList m_roles;

    TopologyMap m_topology;
};

} /* namespace bento */

#endif /* TOPOLOGY_H_ */
