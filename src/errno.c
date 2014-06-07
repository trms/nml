#include "nml.h"

// retrieve the current errno
// http://nanomsg.org/v0.3/nn_errno.3.html
int l_errno(lua_State* L)
{
	lua_pushnumber(L, nn_errno());
	return 1;
}