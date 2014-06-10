#include "nml.h"

// remove an endpoint from a socket
// http://nanomsg.org/v0.3/nn_shutdown.3.html
int l_shutdown(lua_State* L)
{
	luaL_checktype(L, P1, LUA_TNUMBER); // socket
	luaL_checktype(L, P2, LUA_TNUMBER); // eid

	lua_pushnumber(L,nn_shutdown((int)lua_tonumber(L, P1), (int)lua_tonumber(L, P2)));
	return 1;
}