# Main LunaPaak CMake file

project (LunaPaak CXX)





set(SRCS
	LunaPaak.cpp
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
