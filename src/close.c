#include "nml.h"

// close an SP socket
// http://nanomsg.org/v0.3/nn_close.3.html
int l_close(lua_State* L)
{
		// socket
	luaL_checktype(L, P1, LUA_TNUMBER);

	lua_pushnumber(L, nn_close((int)lua_tonumber(L, P1)));
	return 1;
}