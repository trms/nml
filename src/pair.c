#include "socket.h"
#include "nml.h"
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

//NN_PAIR

//uses nml_socket_methods
//assumes that self (socket ud) is in first postion
//table is in second (optional)
//ignores properties that it does not find a handler for.

int nml_pair (lua_State *L) 
{
 	return generic_socket_constructor(L, NN_PAIR, "pair", NULL, nml_std_socket_methods);
}