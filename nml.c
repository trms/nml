// nanomsg.c
#include <stdlib.h>

#include "nml.h"

/* TODO:

We need a message object that defines allocation, buffer and free methods. 
Default should be the NN_MSG, void ** buf, and nn_freemsg.

We also need to let those messages handle:

serialize: translation of C into wire bits.
deserialize: translation of wire bits into C.
decoding: reading of C into lua.
encoding: reading from lua into C.

TODO: Figure out what file descriptors are about and implement them?

*/
static int nml_get_symbols(lua_State *);
static int nml_socket (lua_State *);
static int nml_term (lua_State * );
//const struct nml_socket_methods;

static const struct luaL_Reg nml_funcs [] = {
	{"new", nml_socket},
	{"term", nml_term},
	{"get_symbols", nml_get_symbols},
	{NULL, NULL}

};


//#include "socket.c"

/*

*int nn_socket (int 'domain', int 'protocol');*

domains:

*AF_SP*::
Standard full-blown SP socket.

*AF_SP_RAW*::
Raw SP socket. Raw sockets omit the end-to-end functionality found in AF_SP
sockets and thus can be used to implement intermediary devices in SP topologies.


protocol: (type socket)
*/

/* 	For the lua interface, we're going to flip the arguments so that 
	domain is second and, if ommitted, defaults to AF_SP.
	Otherwise, if "raw" is received, then we'll make an AF_SP_RAW socket.
*/


static int nml_socket (lua_State *L) {
	int domain = AF_SP;
	int protocol = 0;
	int i;
	const char * domain_arg;
	const char * protocol_arg;
	nml_socket_ud * self;

	lua_settop(L, 2); // let there be two arguments
	if (lua_isstring(L, 2)) {
		domain_arg = lua_tostring(L,2);
		if (strcmp(domain_arg, "raw") == 0)
			domain = AF_SP_RAW;
	}
	lua_pop(L,1);

	if (!lua_isstring(L,1))
		return luaL_error(L, "expected a string in the first argument");

	protocol_arg = lua_tostring(L, 1);
	

	for(i = 0; i < NML_PROTOCOL_COUNT; i++ )
		if (strcmp(protocol_arg, NML_PROTOCOL_STRINGS[i]) == 0){
			protocol = NML_NN_PROTOCOLS[i];
			break;
		}

	if (!protocol)
		return luaL_error(L, "could not create a socket. '%s' protocol wasn't found.", protocol_arg);

	self = (nml_socket_ud *) lua_newuserdata(L, sizeof(nml_socket_ud));
	luaL_getmetatable(L, NML_SOCKET_TYPE);
	lua_setmetatable(L, -2);

	self->socket = nn_socket(domain, protocol);
	if (self->socket >= 0) {
		self->len = -1;
		self->L = L;
		
		lua_newtable(L);
		//end_points table. empty, for now.
		lua_newtable(L);
		lua_setfield(L, -2, "end_points");
		lua_setuservalue(L, -2);
	}
	else {
		return nml_error(L, "nanomsg error");
	}
	

	return 1;

}


static int nml_term (lua_State * L) {
	nn_term();
	return 0;
}

static int nml_get_symbols(lua_State * L) {
	int i = 0;
	int current_value;
	const char * symbol;
	do {

		symbol = nn_symbol(i, &current_value);
		++i;
		printf("'%s' = %d\n", symbol, current_value);
	} while (symbol);
	return 1;
}
/* TODO: CLEANUP CODE
Remove bound endpoints.
Clean up buffer.

Call this function on __gc
*/

/*
static int nml_cleanup(lua_State * L) {
	nml_socket_ud * self;
	self = (nml_socket_ud *) luaL_checkudata(L, 1,NML_SOCKET_TYPE);
}
*/



#ifdef _WIN32
 __declspec(dllexport) int luaopen_nml(lua_State *L) {
#else
 int luaopen_nml(lua_State *L) {
 #endif
 
	//the socket metatable 
	luaL_newmetatable(L, NML_SOCKET_TYPE);
	luaL_setfuncs(L, nml_socket_methods, 0);
	// lua_pushvalue(L,-1);
	// lua_setfield(L, -2, "__index");
	//Pop this off the stack now and make the library metatable.
	lua_pop(L, 1);
	//Now make the library metatable.

	luaL_newmetatable(L, NML_LIBNAME);

	//now the metatable will have this one table as an upvalue.
	luaL_newlib(L, nml_funcs);
	return 1;
}
