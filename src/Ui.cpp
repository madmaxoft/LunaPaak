#include "Ui.h"
#include "Utils.h"
extern "C"
{
	#include <lua.h>
}
#include <Windows.h>






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
		lua_pushstring(L, "Expected a table as the first param");
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
		lua_pushstring(L, "Expected a table as the first param");
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





void Ui::createTable(lua_State * L)
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
