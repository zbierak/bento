/*
 * topology.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#include "topology.h"
#include "parser.h"

#include <iostream>
#include <algorithm>
#include <string>

#include "../exceptions.h"

#include <boost/lexical_cast.hpp>

using namespace std;

namespace bento {

Topology::Topology(const std::string& ownerName, const std::string& topologyFileName)
{
    m_ownerName = ownerName;
    m_ownerPort = 0;

	string error;
	if (!parseTopologyFile(topologyFileName, *this, error))
	{
		throw TopologyException(error);
	}

	// determine whose neighbor am I
	for (TopologyMap::const_iterator it = m_topology.begin(); it != m_topology.end(); ++it)
	{
		if (it->second.find(m_ownerName) != it->second.end())
			m_amNeighbourOf.push_back(it->first);
	}

	// determine which of my neighbors have the same role
	string myRole = this->getOwnerRole();
	for (NodeList::const_iterator it = m_neighbours.begin(); it != m_neighbours.end(); ++it)
	{
		if (this->getRole(*it) == myRole)
			m_sameGroupNeighbours.push_back(*it);
	}
}

Topology::~Topology()
{
}

const Topology::AddressList& Topology::getNeighbourAddresses() const
{
	TopologyMap::const_iterator it = m_topology.find(m_ownerName);
	return it->second;
}

const std::string Topology::getOwnerRole() const
{
	return getRole(m_ownerName);
}

const std::string Topology::getRole(const std::string& node) const
{
	RoleList::const_iterator it = m_roles.find(node);
	if (it == m_roles.end())
		return "";
	return it->second;
}

void Topology::debugPrint() const
{
	cout << "Owner name: " << m_ownerName << endl;

	cout << "All nodes:" << endl;
	for (NodeList::const_iterator it = m_nodeList.begin(); it != m_nodeList.end(); ++it)
	{
		cout << "\t" << *it << endl;
	}

	cout << "Neighbours:" << endl;
	for (NodeList::const_iterator it = m_neighbours.begin(); it != m_neighbours.end(); ++it)
	{
		cout << "\t" << *it << endl;
	}

    cout << "Whole topology:" << endl;
    for (TopologyMap::const_iterator it = m_topology.begin(); it != m_topology.end(); ++it)
    {
    	cout << "\t" << "Neighbours of " << it->first << endl;
    	for (AddressList::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
    	{
    		cout << "\t\t" << it2->first << " with address " << it2->second << endl;
    	}
    }
}

void Topology::updateTopologyMap(const NodeList& nodeList, const TopologyMap& topologyMap, const AddressList& defaultAddresses, const RoleList& roles)
{
	m_nodeList = nodeList;
	m_topology = topologyMap;
	m_roles = roles;

    if (std::find(m_nodeList.begin(), m_nodeList.end(), m_ownerName) == m_nodeList.end())
    {
    	throw TopologyException("Node '" + m_ownerName + "' has not been found in the topology.");
    }

    m_neighbours.clear();
    TopologyMap::iterator it = m_topology.find(m_ownerName);
    if (it == m_topology.end())
    {
    	m_topology.insert(make_pair(m_ownerName, AddressList()));
    }
    else
    {
		for (AddressList::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
	    {
			m_neighbours.push_back(it2->first);
	    }
	}

    AddressList::const_iterator defaultIt = defaultAddresses.find(m_ownerName);
    if (defaultIt == defaultAddresses.end())
    {
    	throw TopologyException("No default address for node '" + m_ownerName + "'.");
    }

    string address = defaultIt->second;

    unsigned delim = address.find_last_of(':');
    if (delim == string::npos)
    {
    	throw TopologyException("Default address should be in a form of <host>:<port> (exception while analyzing "+address+" for "+m_ownerName+")");
    }

    m_ownerHost = address.substr(0, delim);
    m_ownerPort = boost::lexical_cast<unsigned>(address.substr(delim+1));
}


// Find out the order number in the group delimited by all neighbors of that
// node that share the same role. The sequence number is determined by the
// alphabetical order of the names in such group. Might sound fishy, but is
// handy sometimes.
const unsigned Topology::getOwnerOrderNumber() const
{
	vector<string> sameGroupNodes(m_sameGroupNeighbours);
	sameGroupNodes.push_back(m_ownerName);

	std::sort(sameGroupNodes.begin(), sameGroupNodes.end());

	for (unsigned i=0; i<sameGroupNodes.size(); i++)
	{
		if (sameGroupNodes[i] == m_ownerName)
			return i;
	}

	// will never happen, for the sake of compiler let's add it though
	return 0;
}


// Get the name of the node placed on the given position in the ordering with
// regard to other neighbors. Similarly to the function above we create a list
// including the owner name and its neighbors and sort it according to node name.
// This function returns the node name located on the <orderNumber> position in
// that list. Proves handy, when you need to designate one of the machines in
// some way (for instance, the one with the lowest order number), but don't
// know what is its name.
const std::string Topology::getNodeWithOrderNumber(unsigned orderNumber) const
{
	vector<string> sameGroupNodes(m_sameGroupNeighbours);
	sameGroupNodes.push_back(m_ownerName);
	std::sort(sameGroupNodes.begin(), sameGroupNodes.end());

	if (orderNumber >= sameGroupNodes.size())
		throw GeneralException("Request was issued to extract node with order number "+boost::lexical_cast<string>(orderNumber) +
				", while the group contains only "+boost::lexical_cast<string>(sameGroupNodes.size())+" nodes.");

	return sameGroupNodes[orderNumber];
}

} /* namespace bento */
