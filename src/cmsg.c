#include "nml.h"

// access control information
// http://nanomsg.org/v0.3/nn_cmsg.3.html
int l_cmsg(lua_State* L)
{
	return pushError(L, _T("not implemented"));
}