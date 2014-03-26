/*
 * IMessageInterceptor.h
 *
 *  Created on: Mar 26, 2014
 *      Author: zbierak
 */

#ifndef IMESSAGEINTERCEPTER_H_
#define IMESSAGEINTERCEPTER_H_

#include <boost/shared_ptr.hpp>

namespace bento {

/*
 * Allows to "steal" an incoming message to the node. The onMessage function
 * should return true if the message has been intercepted and false otherwise.
 */
class IMessageIntercepter
{
public:
	virtual ~IMessageIntercepter() {}
	virtual bool onMessage(const std::string& from, const int32_t type, const std::string& msg) = 0;
};

typedef boost::shared_ptr<IMessageIntercepter> IMessageIntercepterPtr;

} /* namespace bento */

#endif /* IMESSAGEINTERCEPTER_H_ */
