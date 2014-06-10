#include "nml.h"

// set a socket option
// http://nanomsg.org/v0.3/nn_setsockopt.3.html
int l_setsockopt(lua_State* L)
{
	int iValue;

	luaL_checktype(L, P1, LUA_TNUMBER); // socket
	luaL_checktype(L, P2, LUA_TNUMBER); // level
	luaL_checktype(L, P3, LUA_TNUMBER); // option type
	luaL_checktype(L, P4, LUA_TNUMBER); // option value

	iValue = (int)lua_tonumber(L, P4);

	lua_pushnumber(L, nn_setsockopt((int)lua_tonumber(L, P1), (int)lua_tonumber(L, P2), (int)lua_tonumber(L, P3), &iValue, sizeof(iValue)));
	return 1;
}