# Build instructions for the supporting libraries
# All libraries already have their CMakeLists.txt, so those are called, in the correct order




add_subdirectory(zlib)
add_subdirectory(lua)
add_subdirectory(luafilesystem)
add_subdirectory(lzlib)
add_subdirectory(luasocket)
add_subdirectory(lsqlite3)
add_subdirectory(lua-cjson)
add_subdirectory(fmt)
add_subdirectory(LuaSimpleWinHttp)
