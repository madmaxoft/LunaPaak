#include <iostream>
extern "C"
{
	#include <zlib.h>
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
	#include <lfs.h>
	
	// lzlib fwd:
	extern int luaopen_zlib(lua_State *);

	// luasocket fwd:
	extern void luaopen_luasocket(lua_State * L);

	// lsqlite3 fwd:
	extern int luaopen_lsqlite3(lua_State * L);
}





int main(int argc, char * argv[])
{
	static const char luaCode[] = R"(
	print("hello, world")
	lfs = require("lfs")
	zlib = require("zlib")
	skt = require("socket")
	http = require("socket.http")
	sqlite = require("sqlite3")
	print("zlib = " .. tostring(zlib))
	print("skt = " .. tostring(skt))
	print("Attempting socket connection to blackbox...")
	local client = skt.connect("blackbox.xoft.cz", 80)
	print("client = " .. tostring(client))
	client:send("GET / HTTP/1.0\r\nHost: blackbox.xoft.cz\r\n\r\n")
	print(client:receive("*a"))
	print(http.request("http://post.cz"))
	print("All done")
	)";

	lua_State * L = luaL_newstate();
	luaL_openlibs(L);
	luaL_requiref(L, "lfs", &luaopen_lfs, false);
	lua_pop(L, 1);
	luaL_requiref(L, "zlib", &luaopen_zlib, false);
	lua_pop(L, 1);
	luaopen_luasocket(L);
	lua_pop(L, 1);
	luaL_requiref(L, "sqlite3", &luaopen_lsqlite3, false);
	lua_pop(L, 1);

	// TODO

	// Test the Lua interface:
	luaL_loadbuffer(L, luaCode, sizeof(luaCode) - 1, "main");
	auto s = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (s != 0)
	{
		std::cerr << "Can't load test script: " << lua_tostring(L, -1);
		lua_pop(L, 1);
		return 1;
	}

	return 0;
}

