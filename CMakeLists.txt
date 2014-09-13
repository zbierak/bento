cmake_minimum_required(VERSION 2.6)
project(bento) 

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option (BENTO_RELEASE "Build a release version of bento" ON) 

if (BENTO_RELEASE)
	add_definitions(-DLOG_LEVEL=LOG_LEVEL_WARN)
	set(CMAKE_BUILD_TYPE Release)
else(BENTO_RELEASE)
	add_definitions(-DLOG_LEVEL=LOG_LEVEL_ALL)
endif(BENTO_RELEASE)

option (BENTO_USE_HASH_FOR_GATHER "Use hash function for gather message storage" ON)
if (BENTO_USE_HASH_FOR_GATHER)
	add_definitions(-DUSE_HASH_FOR_GATHER)
	find_package(CryptoPP REQUIRED)
	include_directories(${CRYPTOPP_INCLUDE_DIR})
endif(BENTO_USE_HASH_FOR_GATHER)
 

# --- FIND BOOST
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.42.0 COMPONENTS thread system)

if(!Boost_FOUND)
	error("Boost (at least 1.42.0) is required")
endif()

include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(src)

