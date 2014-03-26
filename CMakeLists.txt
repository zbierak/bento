cmake_minimum_required(VERSION 2.6)
project(bento) 

option (BENTO_RELEASE "Build a release version of bento" ON) 

if (BENTO_RELEASE)
	add_definitions(-DLOG_LEVEL=LOG_LEVEL_WARN)
	set(CMAKE_BUILD_TYPE Release)
else(BENTO_RELEASE)
	add_definitions(-DLOG_LEVEL=LOG_LEVEL_ALL)
endif(BENTO_RELEASE)

add_subdirectory(src)

