#include "nml.h"

// get a socket option
// http://nanomsg.org/v0.3/nn_getsockopt.3.html
int l_getsockopt(lua_State* L)
{
	int iValue;
	size_t szValueSize=sizeof(int);

	luaL_checktype(L, P1, LUA_TNUMBER);
	luaL_checktype(L, P2, LUA_TNUMBER);
	luaL_checktype(L, P3, LUA_TNUMBER);

	lua_pushnumber(L, nn_getsockopt((int)lua_tonumber(L, P1), (int)lua_tonumber(L, P2), (int)lua_tonumber(L, P3), &iValue, &szValueSize));

	if (lua_tonumber(L, -1)!=-1)
		lua_pushnumber(L, iValue);
	else
		lua_pushnil(L);
	return 2;
}