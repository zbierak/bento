add_executable(version
	version.cpp
) 

target_link_libraries(version bento)

add_executable(echo
	echo.cpp
) 

target_link_libraries(echo bento)
