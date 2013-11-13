/*
 * logger.h
 *
 *  Created on: Nov 6, 2013
 *      Author: zbierak
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>

namespace bento {

const char* loggerSanitizeFileName(const char*);

#define LOG_DEBUG(format, ARG...) \
	printf("[DEBUG] " format " [%s@%s:%d]\n", ##ARG, __func__, loggerSanitizeFileName(__FILE__), __LINE__);

#define LOG_INFO(format, ARG...) \
	printf("[INFO]  " format " [%s@%s:%d]\n", ##ARG, __func__, loggerSanitizeFileName(__FILE__), __LINE__);

#define LOG_WARN(format, ARG...) \
	printf("[WARN]  " format " [%s@%s:%d]\n", ##ARG, __func__, loggerSanitizeFileName(__FILE__), __LINE__);

#define LOG_ERROR(format, ARG...) \
	printf("[ERROR] " format " [%s@%s:%d]\n", ##ARG, __func__, loggerSanitizeFileName(__FILE__), __LINE__);

} /* namespace bento */

#endif /* LOGGER_H_ */
