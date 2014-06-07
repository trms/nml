#include "nml.h"

// convert an error number into human-readable string
// http://nanomsg.org/v0.3/nn_strerror.3.html
int l_strerror(lua_State* L)
{
	if (lua_type(L, 1)==LUA_TNUMBER) {
		lua_pushstring(L, nn_strerror((int)lua_tonumber(L, 1)));
		return 1;
	}
	return 0;
}