#include "nml.h"

// remove an endpoint from a socket
// http://nanomsg.org/v0.3/nn_shutdown.3.html
int l_shutdown(lua_State* L)
{
	if (isSocket(L, P1)==0)
		return pushError(L, g_achInvalidSocketParameter);

	if (lua_type(L, P2)!=LUA_TNUMBER)
		return pushError(L, _T("expected a number endpoint parameter"));

	if (nn_shutdown((int)lua_tonumber(L, P1), (int)lua_tonumber(L, P2))==-1)
		return pushErrorNo(L);

	lua_pushnumber(L, 0);
	return 1;
}