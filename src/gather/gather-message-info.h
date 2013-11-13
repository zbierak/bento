/*
 * gather-message-info.h
 *
 *  Created on: Nov 13, 2013
 *      Author: zbierak
 */

#ifndef GATHER_MESSAGE_H_
#define GATHER_MESSAGE_H_

#include <boost/unordered_set.hpp>

namespace bento {

class GatherMessageInfo {
public:
	GatherMessageInfo();
	virtual ~GatherMessageInfo();

	unsigned addSender(const std::string& sender);
private:
	boost::unordered_set<std::string> m_senders;
};

} /* namespace bento */

#endif /* GATHER_MESSAGE_H_ */
