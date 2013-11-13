/*
 * topology.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#include "topology.h"

#include <algorithm>

#include "../exceptions.h"

using namespace std;

namespace bento {

Topology::Topology(const std::string& ownerName)
{
    m_ownerName = ownerName;

    // TODO: this should be read from a file (a custom parser perhaps?)
    m_nodeList.push_back("node1");
    m_nodeList.push_back("node2");
    m_nodeList.push_back("node3");

    // check if owner name is a known node name
    if (std::find(m_nodeList.begin(), m_nodeList.end(), ownerName) == m_nodeList.end())
    {
    	throw GeneralException("Node '" + ownerName + "' has not been found in the topology.");
    }

    // we assume all-to-all for now
    for (NodeList::const_iterator it = m_nodeList.begin(); it != m_nodeList.end(); ++it)
    {
    	if (*it != ownerName)
    		m_neighbours.push_back(*it);
    }

    AddressList node1Neighbours, node2Neighbours, node3Neighbours;
    node1Neighbours.push_back(make_pair("node2", "localhost:2014"));
    node1Neighbours.push_back(make_pair("node3", "localhost:2015"));
    node2Neighbours.push_back(make_pair("node1", "localhost:2013"));
    node2Neighbours.push_back(make_pair("node3", "localhost:2015"));
    node3Neighbours.push_back(make_pair("node1", "localhost:2013"));
    node3Neighbours.push_back(make_pair("node2", "localhost:2014"));

    m_topology.insert(make_pair("node1", node1Neighbours));
    m_topology.insert(make_pair("node2", node2Neighbours));
    m_topology.insert(make_pair("node3", node3Neighbours));
}

Topology::~Topology()
{
}

const Topology::AddressList& Topology::getNeighbourAddresses()
{
	TopologyMap::const_iterator it = m_topology.find(m_ownerName);
	return it->second;
}

} /* namespace bento */
