add_library(bento 
	node.cpp
	version.cpp
	topology/incoming-registry.cpp
	topology/sender.cpp
	topology/topology.cpp
	zmq-wrappers/zmq-helpers.cpp
	zmq-wrappers/zmq-inproc.cpp
) 

install(DIRECTORY ./ DESTINATION include/bento
	FILES_MATCHING PATTERN "*.h")

install(TARGETS bento DESTINATION lib)
