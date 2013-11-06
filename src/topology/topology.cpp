/*
 * topology.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#include "topology.h"

using namespace std;

namespace bento {

Topology::Topology(const std::string& ownerName)
{
    m_ownerName = ownerName;

    // TODO: think about what happens when ownerName is not defined in our topology?
}

Topology::~Topology()
{
}

} /* namespace bento */
