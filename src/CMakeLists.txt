add_library(bento 
	version.cpp
) 

install(FILES 
		version.h 
	DESTINATION include/bento
)

install(TARGETS bento DESTINATION lib)
