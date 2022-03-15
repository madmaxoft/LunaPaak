#pragma once





// fwd:
struct lua_State;





namespace Ui
{
	/** Creates the "lunapak.ui" table of functions on top of the stack.
	Used for registering the ui subsystem at the program startup. */
	void createTable(lua_State * L);
}
