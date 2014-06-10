#include "nml.h"

// add a remote endpoint to the socket
// http://nanomsg.org/v0.3/nn_connect.3.html
int l_connect(lua_State* L)
{
	// socket
	luaL_checktype(L, P1, LUA_TNUMBER);

	// address
	luaL_checktype(L, P2, LUA_TSTRING);

	lua_pushnumber(L, nn_connect((int)lua_tonumber(L, P1), lua_tostring(L, P2)));
	return 1;
}