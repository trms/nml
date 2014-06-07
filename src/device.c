#include "nml.h"

// start a device
// http://nanomsg.org/v0.3/nn_device.3.html
int l_device(lua_State* L)
{
	int iResult;

	if (lua_type(L, P1)==LUA_TNUMBER) {
		if (lua_type(L, P2)==LUA_TNUMBER) {
			iResult = nn_device((int)lua_tonumber(L, P1), (int)lua_tonumber(L, P2));
			if (iResult==-1)
				return pushErrorNo(L);
			lua_pushnumber(L, iResult);
			return 1;
		}
	}
	return 0;
}