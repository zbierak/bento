/*
 * gather-message-info.cpp
 *
 *  Created on: Nov 13, 2013
 *      Author: zbierak
 */

#include "gather-message-info.h"

namespace bento {

GatherMessageInfo::GatherMessageInfo()
{
}

GatherMessageInfo::~GatherMessageInfo()
{
}

unsigned GatherMessageInfo::addSender(const std::string& sender)
{
	m_senders.insert(sender);
	return m_senders.size();
}


} /* namespace bento */
