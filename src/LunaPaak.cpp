#include <iostream>
#include <sstream>
#include <ShlObj.h>
#include "RegistryKey.h"
#include "Utils.h"
#include "Ui.h"
extern "C"
{
	#include <zlib.h>
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
	#include <lfs.h>
	#include <LuaSimpleWinHttp.h>
	
	// lzlib fwd:
	extern int luaopen_zlib(lua_State *);

	// luasocket fwd:
	extern void luaopen_luasocket(lua_State * L);

	// lsqlite3 fwd:
	extern int luaopen_lsqlite3(lua_State * L);

	// lua-cjson fwd:
	extern int luaopen_cjson(lua_State *l);
	extern int luaopen_cjson_safe(lua_State *l);
}





extern "C" int luaopen_lunapaak(lua_State * L)
{
	lua_newtable(L);
	Ui::createTable(L);
	lua_setfield(L, -2, "ui");
	/*
	lua_pushcfunction(L, lunapaak_error);
	lua_setfield(L, -2, "error");
	lua_pushcfunction(L, lunapaak_assert);
	lua_setfield(L, -2, "assert");
	*/
	return 1;
}





/** Dumps the contents of the Lua stack to the specified ostream. */
static void dumpLuaStack(lua_State * L, std::ostream & aDest)
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
static void dumpLuaTraceback(lua_State * L, std::ostream & aDest)
{
	luaL_traceback(L, L, "Stack trace: ", 0);
	aDest << lua_getstring(L, -1).c_str() << std::endl;
	lua_pop(L, 1);
	return;
}





/** Reports the error message, and the stack trace, to a UI window. */
static void reportError(lua_State * L, const std::string & aErrorMsg)
{
	std::stringstream ss;
	ss << "Caught an error in the script:\n";
	ss << aErrorMsg;
	if (L != nullptr)
	{
		ss << "\n\n";
		dumpLuaTraceback(L, ss);
	}
	MessageBoxA(NULL, ss.str().c_str(), "LunaPaak", MB_OK | MB_ICONERROR);
}





/** Called by Lua when it encounters an unhandler error in the script file. */
extern "C" int errorHandler(lua_State * L)
{
	auto err = lua_getstring(L, -1);
	std::cerr << "Caught an error: " << err << std::endl;
	dumpLuaStack(L, std::cerr);
	reportError(L, err);
	exit(1);
}





/** Sets the old-style association of .luna files to execute aCommand.
Old-style is used by Windows up to Win7, where programs can change the association directly. */
static void setInterpreterOldStyleAssociation(const std::wstring & aCommand)
{
	try
	{
		// Set up the association in the pre-win8 way:
		RegistryKey::openUserHive(L"Software\\Classes\\Luna.Script.1").setDefaultValue(L"Luna script");
		RegistryKey::openUserHive(L"Software\\Classes\\Luna.Script.1\\shell").setDefaultValue(L"open");
		RegistryKey::openUserHive(L"Software\\Classes\\Luna.Script.1\\shell\\open\\command").setDefaultValue(aCommand.c_str());
		RegistryKey::openUserHive(L"Software\\Classes\\.luna").setDefaultValue(L"Luna.Script.1");

		// Also update possible previous auto-registration made by Windows shell:
		RegistryKey::openUserHive(L"Software\\Classes\\luna_auto_file\\shell\\open\\command").setDefaultValue(aCommand.c_str());
	}
	catch (const std::exception & exc)
	{
		std::cerr << "Failed to register file type association: " << exc.what() << std::endl;
	}
}





/** Removes the new-style association of .luna files.
This is used to replace the wrong association created by user manually, before LunaPaak handled its own associactions.
This is called only once, automatically; the user can force-call this again using the "--fix" parameter. */
static void removeNewStyleAssociation()
{
	try
	{
		RegistryKey::openUserHive(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts").deleteKey(L".luna");
		RegistryKey::openUserHive(L"Software\\Classes\\Luna.Script.1").setValue(L"HasRemovedNewStyle", L"true");
	}
	catch (const std::exception & exc)
	{
		std::cerr << "Failed to remove previous file type association: " << exc.what() << std::endl;
	}
}





/** Removes the new-style association of .luna files, but only if it hasn't been removed before.
This is used to replace the wrong association created by user manually, before LunaPaak handled its own associactions. */
static void removeNewStyleAssociationOnce()
{
	// Query the stored flag for "only once":
	try
	{
		if (RegistryKey::openUserHive(L"Software\\Classes\\Luna.Script.1").getValueString(L"", L"HasRemovedNewStyle") == L"true")
		{
			// Already has been removed, bail out
			return;
		}
	}
	catch (const RegistryKey::RegistryValueNotFoundException &)
	{
		// Not removed yet, remove now
	}
	catch (const std::exception & exc)
	{
		std::cerr << "Failed to remove previous file type association: " << exc.what() << std::endl;
		return;
	}

	removeNewStyleAssociation();
}





/** Registers this executable to handle .luna files, through the Windows registry. */
static void fixFileAssociations()
{
	wchar_t programName[2 * MAX_PATH];
	GetModuleFileNameW(nullptr, programName, sizeof(programName) / sizeof(*programName) - 1);
	std::wstring cmd;
	if (programName[0] != L'"')
	{
		cmd.push_back(L'"');
	}
	cmd.append(programName);
	if (programName[0] != L'"')
	{
		cmd.push_back(L'"');
	}
	cmd.append(L" \"%0\" %*");
	setInterpreterOldStyleAssociation(cmd);
	removeNewStyleAssociationOnce();
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST , nullptr, nullptr);
}





int main(int argc, char * argv[])
{
	fixFileAssociations();
	if (argc <= 1)
	{
		return 0;
	}

	if ((argc == 2) && (strcmp(argv[1], "--fix") == 0))
	{
		removeNewStyleAssociation();
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST , nullptr, nullptr);
		return 0;
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
	luaL_requiref(L, "sqlite3", &luaopen_lsqlite3, false);
	lua_pop(L, 1);
	luaL_requiref(L, "lunapaak", &luaopen_lunapaak, true);
	lua_pop(L, 1);
	luaL_requiref(L, "cjson", &luaopen_cjson, false);
	lua_pop(L, 1);
	luaL_requiref(L, "cjson.safe", &luaopen_cjson, false);
	luaopen_cjson_safe(L);
	lua_pop(L, 1);
	luaopen_LuaSimpleWinHttp(L);
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
	auto status = luaL_loadfile(L, argv[1]);
	if (status != LUA_OK)
	{
		errorHandler(L);
	}
	for (int i = 2; i < argc; ++i)
	{
		lua_pushstring(L, argv[i]);
	}
	lua_pcall(L, argc - 2, LUA_MULTRET, 1);
	return 0;
}

