/*
 * exceptions.h
 *
 *  Created on: Nov 6, 2013
 *      Author: zbierak
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <stdexcept>

namespace bento {

class GeneralException: public std::exception {
public:
	GeneralException(const std::string& msg): m_msg(msg) {}
	~GeneralException() throw () {}

	const char* what() const throw() { return m_msg.c_str(); }
private:
	std::string m_msg;
};

class TopologyException: public GeneralException {
public:
	TopologyException(const std::string& msg): GeneralException(msg) {}
};

} /* namespace bento */

#endif /* EXCEPTIONS_H_ */
