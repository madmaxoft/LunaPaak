#include <Windows.h>
#include <iostream>
#include <sstream>
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





#define ARRAYCOUNT(X) (sizeof(X) / sizeof(*(X)))





/** Returns the value at the specified stack index as a std::string. */
std::string lua_getstring(lua_State * L, int aIndex)
{
	size_t len;
	auto s = lua_tolstring(L, aIndex, &len);
	return std::string(s, len);
}





std::string lua_getstringfield(lua_State * L, int aTableIndex, const char * aFieldName)
{
	lua_getfield(L, aTableIndex, aFieldName);
	std::string res = lua_getstring(L, -1);
	lua_pop(L, 1);
	return res;
}





// lunapaak.ui.msgbox() function implementation
extern "C" int lunapaak_ui_msgbox(lua_State * L)
{
	// Function signature:
	// lunapaak.ui.msgbox("caption", "text", [{options}]);
	if (
		!lua_isstring(L, 1) ||
		!lua_isstring(L, 2)
	)
	{
		lua_pushstring(L, "Parameter type mismatch, expected at least two strings");
		return lua_error(L);
	}

	// Process the options:
	UINT flags = 0;
	if (lua_istable(L, 3))
	{
		auto buttons = lua_getstringfield(L, 3, "buttons");
		if (buttons == "ok")
		{
			flags = MB_OK;
		}
		else if (buttons == "okcancel")
		{
			flags = MB_OKCANCEL;
		}
		else if (buttons == "yesno")
		{
			flags = MB_YESNO;
		}
		else if (buttons == "yesnocancel")
		{
			flags = MB_YESNOCANCEL;
		}
		else if (buttons == "abortretryignore")
		{
			flags = MB_ABORTRETRYIGNORE;
		}
		else if (buttons == "canceltryagaincontinue")
		{
			flags = MB_CANCELTRYCONTINUE;
		}
		else if (buttons == "retrycancel")
		{
			flags = MB_RETRYCANCEL;
		}

		auto icon = lua_getstringfield(L, 3, "icon");
		if ((icon == "error") || (icon == "stop") || (icon == "hand"))
		{
			flags |= MB_ICONERROR;
		}
		else if ((icon == "warning") || (icon == "exclamation"))
		{
			flags |= MB_ICONWARNING;
		}
		else if ((icon == "information") || (icon == "asterisk"))
		{
			flags |= MB_ICONINFORMATION;
		}
		else if (icon == "question")
		{
			flags |= MB_ICONQUESTION;
		}
	}

	// Show the message box:
	auto caption = lua_getstring(L, 1);
	auto text = lua_getstring(L, 2);
	auto res = MessageBoxA(nullptr, text.c_str(), caption.c_str(), flags);
	lua_pushinteger(L, res);
	return 1;
}





// lunapaak.ui.getopenfilename() function implementation
extern "C" int lunapaak_ui_getopenfilename(lua_State * L)
{
	if (!lua_istable(L, 1))
	{
		lua_pushstring(L, "Expected a string as the first param");
		return lua_error(L);
	}

	auto title = lua_getstringfield(L, 1, "title");
	auto filter = lua_getstringfield(L, 1, "filter");
	auto filename = lua_getstringfield(L, 1, "filename");
	auto initialdir = lua_getstringfield(L, 1, "initialdir");
	// TODO: auto multiselect = lua_getbooleanfield(L, 1, "multiselect");

	OPENFILENAMEA ofn;
	char buf[MAX_PATH * 4];
	strncpy(buf, filename.c_str(), ARRAYCOUNT(buf));
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrTitle = title.c_str();
	ofn.lpstrFilter = filter.c_str();
	ofn.lpstrFile = buf;
	ofn.lpstrInitialDir = initialdir.empty() ? nullptr : initialdir.c_str();
	ofn.nMaxFile = ARRAYCOUNT(buf);
	ofn.Flags = OFN_ENABLESIZING;
	if (!GetOpenFileNameA(&ofn))
	{
		if (GetLastError() > 0)
		{
			// An error - return a nil and number description
			lua_pushnil(L);
			lua_pushinteger(L, GetLastError());
			return 2;
		}
		// User cancelled, return nothing
		return 0;
	}
	lua_pushstring(L, ofn.lpstrFile);
	return 1;
}





// lunapaak.ui.getsavefilename() function implementation
extern "C" int lunapaak_ui_getsavefilename(lua_State * L)
{
	if (!lua_istable(L, 1))
	{
		lua_pushstring(L, "Expected a string as the first param");
		return lua_error(L);
	}

	auto title = lua_getstringfield(L, 1, "title");
	auto filter = lua_getstringfield(L, 1, "filter");
	auto filename = lua_getstringfield(L, 1, "filename");
	auto initialdir = lua_getstringfield(L, 1, "initialdir");

	OPENFILENAMEA ofn;
	char buf[MAX_PATH * 4];
	strncpy(buf, filename.c_str(), ARRAYCOUNT(buf));
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrTitle = title.c_str();
	ofn.lpstrFilter = filter.c_str();
	ofn.lpstrFile = buf;
	ofn.lpstrInitialDir = initialdir.empty() ? nullptr : initialdir.c_str();
	ofn.nMaxFile = ARRAYCOUNT(buf);
	ofn.Flags = OFN_ENABLESIZING;
	if (!GetSaveFileNameA(&ofn))
	{
		if (GetLastError() > 0)
		{
			// An error - return a nil and number description
			lua_pushnil(L);
			lua_pushinteger(L, GetLastError());
			return 2;
		}
		// User cancelled, return nothing
		return 0;
	}
	lua_pushstring(L, ofn.lpstrFile);
	return 1;
}





/*
extern "C" int lunapaak_error(lua_State * L)
{
	std::stringstream ss;
	dumpLuaTraceback(L, ss);
}
*/





/** Creates the "lunapak.ui" table of functions on top of the stack. */
void createUiTable(lua_State * L)
{
	lua_newtable(L);
	lua_pushcfunction(L, lunapaak_ui_msgbox);
	lua_setfield(L, -2, "msgbox");
	lua_pushcfunction(L, lunapaak_ui_getopenfilename);
	lua_setfield(L, -2, "getopenfilename");
	lua_pushcfunction(L, lunapaak_ui_getsavefilename);
	lua_setfield(L, -2, "getsavefilename");
	lua_pushinteger(L, IDOK);
	lua_setfield(L, -2, "OK");
	lua_pushinteger(L, IDYES);
	lua_setfield(L, -2, "YES");
	lua_pushinteger(L, IDNO);
	lua_setfield(L, -2, "NO");
	lua_pushinteger(L, IDCANCEL);
	lua_setfield(L, -2, "CANCEL");
	lua_pushinteger(L, IDABORT);
	lua_setfield(L, -2, "ABORT");
	lua_pushinteger(L, IDRETRY);
	lua_setfield(L, -2, "RETRY");
	lua_pushinteger(L, IDCONTINUE);
	lua_setfield(L, -2, "CONTINUE");
	lua_pushinteger(L, IDIGNORE);
	lua_setfield(L, -2, "IGNORE");
	lua_pushinteger(L, IDTRYAGAIN);
	lua_setfield(L, -2, "TRYAGAIN");
}





extern "C" int luaopen_lunapaak(lua_State * L)
{
	lua_newtable(L);
	createUiTable(L);
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
	luaL_traceback(L, L, "Stack trace: ", 0);
	aDest << lua_getstring(L, -1).c_str() << std::endl;
	lua_pop(L, 1);
	return;
}





/** Reports the error message, and the stack trace, to a UI window. */
void reportError(lua_State * L, const std::string & aErrorMsg)
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





int main(int argc, char * argv[])
{
	if (argc <= 1)
	{
		reportError(nullptr, "This program is a Lua code interpreter. Run it with the filename of the script to run as a parameter.");
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

