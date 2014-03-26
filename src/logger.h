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

#define LOG_LEVEL_ALL   0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_NONE  5

// by default, all logging is enabled (you can change it also in the cmake)
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

const char* loggerSanitizeFileName(const char*);

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ARG...) \
	printf("[DEBUG] " format " [%s@%s:%d]\n", ##ARG, __func__, loggerSanitizeFileName(__FILE__), __LINE__);
#else
#define LOG_DEBUG(format, ARG...);
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(format, ARG...) \
	printf("[INFO]  " format " [%s@%s:%d]\n", ##ARG, __func__, loggerSanitizeFileName(__FILE__), __LINE__);
#else
#define LOG_INFO(format, ARG...);
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(format, ARG...) \
	printf("[WARN]  " format " [%s@%s:%d]\n", ##ARG, __func__, loggerSanitizeFileName(__FILE__), __LINE__);
#else
#define LOG_WARN(format, ARG...);
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ARG...) \
	printf("[ERROR] " format " [%s@%s:%d]\n", ##ARG, __func__, loggerSanitizeFileName(__FILE__), __LINE__);
#else
#define LOG_ERROR(format, ARG...);
#endif

} /* namespace bento */

#endif /* LOGGER_H_ */
