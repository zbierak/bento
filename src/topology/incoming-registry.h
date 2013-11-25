/*
 * incoming_registry.h
 *
 *  Created on: Oct 28, 2013
 *      Author: zbierak
 */

#ifndef INCOMING_REGISTRY_H_
#define INCOMING_REGISTRY_H_

#include <zmq.hpp>
#include <boost/unordered_map.hpp>

namespace bento {

class IncomingRegistry {
public:
    IncomingRegistry() {}
    virtual ~IncomingRegistry() {}

    void addName(const std::string& zmqId, const std::string& name);
    bool getName(const std::string& zmqId, std::string& name);

    bool containsName(const std::string& name);
private:
    typedef boost::unordered_map<std::string, std::string> IncomingMap;
    IncomingMap m_incomingMap;
};

} /* namespace bento */

#endif /* TOPOLOGY_H_ */

