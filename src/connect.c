#include "nml.h"

// add a remote endpoint to the socket
// http://nanomsg.org/v0.3/nn_connect.3.html
int l_connect(lua_State* L)
{
	int iEndpointId;

	if (isSocket(L, P1)==0)
		return pushError(L, g_achInvalidSocketParameter);

	if (lua_type(L, P2)!=LUA_TSTRING)
		return pushError(L, _T("expected a string address parameter"));

	iEndpointId = nn_connect((int)lua_tonumber(L, P1), lua_tostring(L, P2));

	if (iEndpointId==-1)
		return pushErrorNo(L);

	lua_pushnumber(L, iEndpointId);
	return 1;
}