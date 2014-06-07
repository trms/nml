#include "nml.h"

// allocate a message
// http://nanomsg.org/v0.3/nn_allocmsg.3.html
int l_allocmsg(lua_State* L)
{
	// this is called internally by nml
	return pushError(L, _T("not implemented"));
}