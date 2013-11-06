/*
 * logger.cpp
 *
 *  Created on: Nov 6, 2013
 *      Author: zbierak
 */

#include "logger.h"

#include <string>

using namespace std;

namespace bento {

const char* loggerSanitizeFileName(const char* fileName)
{
	string file(fileName);
	size_t slash = file.find_last_of('/');
	if (slash == string::npos)
	{
		return fileName;
	}
	else
	{
		file = file.substr(slash+1);
		return file.c_str();
	}
}

}
