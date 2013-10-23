add_library(bento 
	node.cpp
	version.cpp
	zmq-helpers.cpp
) 

install(FILES 
		node.h
		version.h 
		zmq-context.h
		zmq-helpers.h
	DESTINATION include/bento
)

install(TARGETS bento DESTINATION lib)
