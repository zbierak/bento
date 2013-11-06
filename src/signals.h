/*
 * signals.h
 *
 *  Created on: Oct 23, 2013
 *      Author: zbierak
 */

#ifndef SIGNALS_H_
#define SIGNALS_H_

namespace bento {

/*
 * Following are signals that are used in the internal protocol.
 * One thing you need to have in mind that their contents need to be unique.
 */
const std::string SIGNAL_HELLO = "EHLO";
const std::string SIGNAL_HELLO_OK = "EHLO OK";

} /* namespace bento */

#endif /* SIGNALS_H_ */
