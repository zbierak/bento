cmake_minimum_required(VERSION 2.6)
project(bento-examples) 

# --- FIND ZEROMQ
FIND_LIBRARY(ZEROMQ_LIBRARY
	NAMES libzmq zmq
	PATHS
		${ZEROMQ_LIBRARIES}
	        ${NSCP_LIBRARYDIR}
)
	
FIND_PATH(ZEROMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS
                ${ZEROMQ_INCLUDE_DIRS}
                ${NSCP_INCLUDEDIR}
                ${ZEROMQ_INCLUDE_DIR}
)
	
include_directories(${ZEROMQ_INCLUDE_DIR})

# --- FIND BOOST
# @todo: add some fancy script for detecting boost placement

# --- add targets

add_executable(version
	version.cpp
) 

target_link_libraries(version bento)

add_executable(echo
	echo.cpp
) 

target_link_libraries(echo bento)

add_executable(connect-test
	connect-test.cpp
) 

target_link_libraries(connect-test bento ${ZEROMQ_LIBRARY} boost_thread boost_system)
