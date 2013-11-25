/*
 * incoming_registry.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: zbierak
 */

#include "incoming-registry.h"

using namespace std;

namespace bento {

void IncomingRegistry::addName(const std::string& zmqId, const std::string& name)
{
    m_incomingMap.insert(make_pair(zmqId, name));
}

bool IncomingRegistry::getName(const std::string& zmqId, std::string& name)
{
    IncomingMap::iterator it = m_incomingMap.find(zmqId);
    if (it == m_incomingMap.end())
        return false;

    name = it->second;
    return true;
}

bool IncomingRegistry::containsName(const std::string& name)
{
	for (IncomingMap::const_iterator it = m_incomingMap.begin(); it != m_incomingMap.end(); ++it)
	{
		if (it->second == name)
			return true;
	}
	return false;
}

}
