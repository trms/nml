/***
@module nml.core
*/

// nanomsg.c


#include "nml.h"

/* TODO:

We need a message object that defines allocation, buffer and free methods. 
Default should be the NN_MSG, void ** buf, and nn_freemsg.

We also need to let those messages handle:

serialize: translation of C into wire bits.
deserialize: translation of wire bits into C.
decoding: reading of C into lua.
encoding: reading from lua into C.


*/
/* terminates the library, sending EITERM to all sockets. */
static int nml_term (lua_State * );
/* garbage collector method for nml. */
static int nml_gc (lua_State *);




//function values for nml table. These are not the metamethods.


//metamethods for nml
static const struct luaL_Reg nml_metamethods [] = {
	
	{"bus", nml_bus},
	{"pair", nml_pair},
	{"push", nml_push},
	{"pull", nml_pull},
	{"pub", nml_pub},
	{"sub", nml_sub},
	{"req", nml_req},
	{"rep", nml_rep},
	{"surveyor", nml_surveyor},
	{"respondent", nml_respondent},
	{"freemsg", nml_freemsg},
	{"__gc", nml_gc},
	{"terminate", nml_term},
	{NULL, NULL}
};
///nml Library object
//@type nml

/***
Creates a new nml socket object
@function new
@param[type=string]	protocol	The name of the socket's protocol
@param[opt=false] is_raw		If true, the protocol is a raw socket
@return nml_proto_socket		The new @{nml.socket}.
*/



/***
Terminates the nml library, forcing all sockets to close.
@function terminate
*/
static int nml_term (lua_State * L) {
	//nn_term();
	return 0;
}

/*
Garbage collector for nml library.
*/
static int nml_gc(lua_State * L) {
	nml_term(L);
	return 0;
}



extern "C" __declspec(dllexport) int luaopen_nml_core(lua_State *L) {
	//the socket metatable
	//used by constructors to make sockets.
	luaL_newmetatable(L, NML_SOCKET);
	luaL_setfuncs(L, nml_socket_metamethods, 0);
	lua_pop(L,1);

	//Now make the library metatable.
	luaL_newlib(L, nml_metamethods);
	return 1; //return empty table on stack
}
