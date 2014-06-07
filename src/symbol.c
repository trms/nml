#include "nml.h"

// query the names and values of nanomsg symbols
// http://nanomsg.org/v0.3/nn_symbol.3.html
int l_symbol(lua_State* L)
{
	if (lua_type(L, P1)==LUA_TNUMBER) {
		lua_pushstring(L, nn_symbol((int)lua_tonumber(L, 1), NULL));
		return 1;
	}
	return 0;
}