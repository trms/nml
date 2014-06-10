#include "nml.h"

// start a device
// http://nanomsg.org/v0.3/nn_device.3.html
int l_device(lua_State* L)
{
	// sockets
	luaL_checktype(L, P1, LUA_TNUMBER);
	luaL_checktype(L, P2, LUA_TNUMBER);

	lua_pushnumber(L, nn_device((int)lua_tonumber(L, P1), (int)lua_tonumber(L, P2)));
	return 1;
}