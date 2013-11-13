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
	int lastPos = -1;
	int i = 0;

	while (fileName[i] != 0)
	{
		if (fileName[i] == '/')
			lastPos = i;
		i++;
	}

	return &fileName[lastPos+1];
}

}
