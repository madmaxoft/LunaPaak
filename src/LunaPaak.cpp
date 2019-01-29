#include <Windows.h>
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





extern "C" int luaopen_lunapaak(lua_State * L)
{
	lua_newtable(L);
	// TODO: Provide our own API
	return 1;
}





void reportError(const std::string & aErrorMsg)
{
	std::cerr << aErrorMsg.c_str() << std::endl;
	// MessageBoxA(NULL, aErrorMsg.c_str(), "LunaPaak", MB_OK | MB_ICONERROR);
}





/** Returns the value at the specified stack index as a std::string. */
std::string lua_getstring(lua_State * L, int aIndex)
{
	size_t len;
	auto s = lua_tolstring(L, aIndex, &len);
	return std::string(s, len);
}





/** Dumps the contents of the Lua stack to the specified ostream. */
void dumpLuaStack(lua_State * L, std::ostream & aDest)
{
	aDest << "Lua stack contents:" << std::endl;
	for (int i = lua_gettop(L); i >= 0; --i)
	{
		aDest << "  " << i << "\t";
		aDest << lua_typename(L, lua_type(L, i)) << "\t";
		aDest << lua_getstring(L, i).c_str() << std::endl;
	}
	aDest << "(stack dump completed)" << std::endl;
}





/** Dumps the call stack to the specified ostream. */
void dumpLuaTraceback(lua_State * L, std::ostream & aDest)
{
	luaL_traceback(L, L, "Traceback: ", 0);
	aDest << lua_getstring(L, -1).c_str() << std::endl;
	lua_pop(L, 1);
	return;
}





/** Called by Lua when it encounters an unhandler error in the script file. */
int errorHandler(lua_State * L)
{
	std::cerr << "Caught an error" << std::endl;
	auto err = lua_getstring(L, -1);
	reportError(err);

	// Dump the Lua stack:
	dumpLuaStack(L, std::cerr);

	// Dump the traceback:
	dumpLuaTraceback(L, std::cerr);

	exit(1);
}





int main(int argc, char * argv[])
{
	if (argc <= 1)
	{
		reportError("This program is a Lua code interpreter. Run it with the filename of the script to run as a parameter.");
		return 1;
	}

	// Create a new Lua state:
	lua_State * L = luaL_newstate();

	// Add the libraries:
	luaL_openlibs(L);
	luaL_requiref(L, "lfs", &luaopen_lfs, false);
	lua_pop(L, 1);
	luaL_requiref(L, "zlib", &luaopen_zlib, false);
	lua_pop(L, 1);
	luaopen_luasocket(L);
	lua_pop(L, 1);
	dumpLuaStack(L, std::cerr);
	luaL_requiref(L, "sqlite3", &luaopen_lsqlite3, false);
	lua_pop(L, 1);
	luaL_requiref(L, "lunapaak", &luaopen_lunapaak, true);
	lua_settop(L, 0);  // Trim off all excess values left over by the reg functions

	// Store the args to the script in a separate "arg" global:
	lua_createtable(L, argc - 1, 0);
	for (int i = 0; i < argc; i++)
	{
		lua_pushstring(L, argv[i]);
		lua_rawseti(L, -2, i - 1);
	}
	lua_setglobal(L, "arg");

	// Execute the script file with appropriate args:
	lua_pushcfunction(L, errorHandler);
	for (int i = 2; i < argc; ++i)
	{
		lua_pushstring(L, argv[i]);
	}
	auto status = luaL_loadfile(L, argv[1]);
	if (status == LUA_OK)
	{
		lua_pcall(L, argc - 2, LUA_MULTRET, 1);
	}
	else
	{
		errorHandler(L);
	}
	return 0;
}

