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
	int iEndpointId;

	if (isSocket(L, P1)==0)
		return pushError(L, g_achInvalidSocketParameter);

	if (lua_type(L, P2)!=LUA_TSTRING)
		return pushError(L, _T("expected a string address parameter"));

	iEndpointId = nn_bind((int)lua_tonumber(L, P1), lua_tostring(L, P2));

	if (iEndpointId==-1)
		return pushErrorNo(L);
	
	lua_pushnumber(L, iEndpointId);
	return 1;
}