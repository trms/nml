#include "nml.h"

// fine-grained alternative to nn_send
// http://nanomsg.org/v0.3/nn_sendmsg.3.html
int l_sendmsg(lua_State* L)
{
	return pushError(L, _T("not implemented"));
}