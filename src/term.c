#include "nml.h"

// notify all sockets about process termination
// http://nanomsg.org/v0.3/nn_term.3.html
int l_term(lua_State* L)
{
	nn_term();
	return 0;
}