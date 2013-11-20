/*
 * parser.h
 *
 *  Created on: Nov 13, 2013
 *      Author: zbierak
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "topology.h"

namespace bento {

bool parseTopologyFile(const std::string& file, Topology& result, std::string& error);

} /* namespace bento */

#endif /* PARSER_H_ */
