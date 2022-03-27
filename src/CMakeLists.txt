# Main LunaPaak CMake file

project (LunaPaak CXX)





set(SRCS
	LunaPaak.cpp
	RegistryKey.cpp
	StringUtils.cpp
	Ui.cpp
	Utils.cpp
)





set(HDRS
	RegistryKey.h
	StringUtils.h
	Ui.h
	Utils.h
)





add_executable(LunaPaak
	${SRCS}
	${HDRS}
)




target_link_libraries(LunaPaak
	lua-static
	luafilesystem-static
	lzlib-static
	luasocket-static
	lsqlite3-static
	cjson-static
	LuaSimpleWinHttp-static
)


set(TESTS
	HelloWorld
	Sqlite
	Lfs
	CJson
	LuaSimpleWinHttp
)



foreach(test IN LISTS TESTS)
	add_test(
		NAME ${test}
		COMMAND LunaPaak ${test}.luna
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../tests
	)
endforeach()
