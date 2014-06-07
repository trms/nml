#include "nml.h"

// close an SP socket
// http://nanomsg.org/v0.3/nn_close.3.html
int l_close(lua_State* L)
{
	if (isSocket(L, P1)==0)
		return pushError(L, g_achInvalidSocketParameter);

	if (nn_close((int)lua_tonumber(L, P1))==-1)
		return pushErrorNo(L); // this will push errno

	lua_pushnumber(L, 0);
	return 1;
}