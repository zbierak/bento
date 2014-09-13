/*
 * gather-registry.h
 *
 *  Created on: Nov 13, 2013
 *      Author: zbierak
 */

#ifndef GATHER_REGISTRY_H_
#define GATHER_REGISTRY_H_

#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "gather-message-info.h"

namespace bento {

/*
 * GatherRegistry accumulates messages (for registered message types only) until
 * it obtains at least minMessages messages of a given type from different nodes,
 * each containing exactly the same contents.
 *
 * Note that when exactly the minMessages messages arrive, GatherRegistry notifies
 * of that fact, but when more identical messages arrive, it does not. Oh well,
 * this doesn't make it easier now, does it? Suppose message of type X has set
 * minMessages to 3. Then, when onMessage for message type X with exactly the same
 * contents from different nodes is called, then it will return true only for the
 * third call, and for the fourth it will return false. This is made in order to prevent
 * multiple notifications on the same message.
 */
class GatherRegistry
{
public:
	GatherRegistry();
	virtual ~GatherRegistry();

	/*
	 * Register new message type in the GatherRegistry
	 */
	void registerMessageType(int32_t type, unsigned minMessages);

	/*
	 * Deregister a message type from the registry. Do not use (unless 100% sure)
	 * when some messages have already been received, as it will remove them
	 * permanently.
	 */
	void deregisterMessageType(int32_t type);

	/*
	 * Called when a new message has arrived. Returns true if exactly required number
	 * of messages (including this message) has been obtained and it hasn't returned
	 * true yet for such message earlier on.
	 */
	bool onMessage(const std::string& from, int32_t type, const std::string& msg);

	/*
	 * Cleanup the contents of message registry to save memory. This entirely erases
	 * the log of delivered messages, which might lead to delivering an already delivered
	 * message once again. Only use when you understand this and all possible implications.
	 */
	void cleanup();
private:
	typedef boost::unordered_map<int32_t, unsigned> AmountMap;
	AmountMap m_requiredMinimum;

	typedef boost::unordered_map<std::string, boost::shared_ptr<GatherMessageInfo> > MessageMap;
	typedef boost::unordered_map<int32_t, MessageMap> TypesMap;
	TypesMap m_awaitingMessages;

	typedef boost::unordered_set<std::string> MessageSet;
	MessageSet m_processedMessages;
};

} /* namespace bento */

#endif /* GATHER_REGISTRY_H_ */
