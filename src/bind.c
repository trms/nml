#include "nml.h"

// http://nanomsg.org/v0.3/nn_bind.3.html
/***
add a local endpoint to the socket
@function bind
@param[type=int]	protocol	The name of the socket's protocol
@param[opt=false] is_raw		If true, the protocol is a raw socket
@return nml			The new @{nml} socket.
*/
int l_bind(lua_State* L)
{
	// socket
	luaL_checktype(L, P1, LUA_TNUMBER);

	// address
	luaL_checktype(L, P2, LUA_TSTRING);

	lua_pushnumber(L, nn_bind((int)lua_tonumber(L, P1), lua_tostring(L, P2)));
	return 1;
}