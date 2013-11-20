add_library(bento 
	node.cpp
	logger.cpp
	version.cpp
	gather/gather-message-info.cpp
	gather/gather-registry.cpp
	topology/incoming-registry.cpp
	topology/parser.cpp
	topology/sender.cpp
	topology/topology.cpp
	zmq-wrappers/zmq-helpers.cpp
	zmq-wrappers/zmq-inproc.cpp
) 

install(DIRECTORY ./ DESTINATION include/bento
	FILES_MATCHING PATTERN "*.h")

install(TARGETS bento DESTINATION lib)
