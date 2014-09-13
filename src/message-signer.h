/*
 * message-signer.h
 *
 *  Created on: Nov 26, 2013
 *      Author: zbierak
 */

#ifndef MESSAGE_SIGNER_H_
#define MESSAGE_SIGNER_H_

#include "topology/topology.h"
#include <inttypes.h>

namespace bento {

class IMessageSigner {
public:
	IMessageSigner() {}
	virtual ~IMessageSigner() {}

	virtual void signMessage(const Topology& topology, const std::string& to, const int32_t type, const std::string& msg, std::string& signature) = 0;
	virtual bool verifyMessage(const Topology& topology, const std::string& from, const int32_t type, const std::string& msg, const std::string& signature) = 0;

	inline virtual int32_t getUniqueSignerId() { return -1; }

	inline virtual void cleanup() {}
};

} /* namespace bento */

#endif /* MESSAGE_SIGNER_H_ */
