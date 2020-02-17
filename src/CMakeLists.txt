# Main LunaPaak CMake file

project (LunaPaak CXX)





set(SRCS
	LunaPaak.cpp
	RegistryKey.cpp
	StringUtils.cpp
)





set(HDRS
	RegistryKey.h
	StringUtils.h
)





add_executable(LunaPaak
	${SRCS}
	${HDRS}
)




target_link_libraries(LunaPaak
	zlibstatic
	lua
	luafilesystem
	lzlib
	luasocket
	lsqlite3
)


set(TESTS
	HelloWorld
	Sqlite
	Lfs
)



foreach(test IN LISTS TESTS)
	add_test(
		NAME ${test}
		COMMAND LunaPaak tests/${test}.luna
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIRECTORY}/..
	)
endforeach()
