#include "socket.h"
#include "nml.h"
#include <nn.h>
#include <pair.h>
#include <pipeline.h>

const struct luaL_Reg nml_options_std_set [] = {
	{"linger", nml_set_linger},
	{"send_buffer", nml_set_int},
	{"receive_buffer", nml_set_int},
	{"send_timeout", nml_set_timeout},
	{"receive_timeout", nml_set_timeout},
	{"reconnect_interval", nml_set_int},
	{"reconnect_interval_max", nml_set_reconnect_interval_max},
	{"send_priority", nml_set_int},
	{"ipv4_only", nml_set_ipv4only},
	{"domain", nml_set_read_only},
	{"protocol", nml_set_read_only},
	{"send_fd", nml_set_read_only},
	{"receive_fd", nml_set_read_only},
	{NULL, NULL}
};
const struct luaL_Reg nml_options_std_get [] = {
	{"linger", nml_get_linger},
	{"send_buffer", nml_get_int},
	{"receive_buffer", nml_get_int},
	{"send_timeout", nml_get_timeout},
	{"receive_timeout", nml_get_timeout},
	{"reconnect_interval", nml_get_int},
	{"reconnect_interval_max", nml_get_reconnect_interval_max},
	{"send_priority", nml_get_int},
	{"ipv4_only", nml_get_ipv4only},
	{"domain", nml_get_domain},
	{"protocol", nml_get_protocol},
	//should error nicely when access to non-rcv/snd is tried.
	{"send_fd", nml_getfd},
	{"receive_fd", nml_getfd},
	{NULL, NULL}
};
const struct luaL_Reg nml_sub_options_set [] = {
	{"subscriptions", nml_set_subscribe},
	{NULL, NULL}
};
const struct luaL_Reg nml_sub_options_get [] = {
	{"subscriptions", nml_get_subscribe},
	{NULL, NULL}
};
//__newindex of sub.subscriptions 
const struct luaL_Reg nml_subscriptions_mt [] = {
	{"__newindex", nml_subscribe__newindex},
	{"__index", nml_subscribe__index},
	{"__len", nml_subscribe__len},
	{"__pairs", nml_subscribe__pairs},
	{NULL, NULL}
};
const struct luaL_Reg nml_req_options_set [] = {
	{"resend_request_interval", nml_set_int},
	{NULL, NULL}
};
const struct luaL_Reg nml_req_options_get [] = {
	{"resend_request_interval", nml_get_int},
	{NULL, NULL}
};

const struct luaL_Reg nml_surveyor_options_set [] = {
	{"surveyor_deadline", nml_set_int},
	{NULL, NULL}
};
const struct luaL_Reg nml_surveyor_options_get [] = {
	{"surveyor_deadline", nml_get_int},
	{NULL, NULL}
};
const struct luaL_Reg nml_socket_std [] = {
	{"close", nml_close},
	{"bind", nml_bind},
	{"connect", nml_connect},
	{"receive", nml_recv},
	{"send", nml_send},
	{"shutdown",  nml_shutdown},
	{NULL, NULL}
};
const struct luaL_Reg nml_socket_send_only [] = {
	{"close", nml_close},
	{"bind", nml_bind},
	{"connect", nml_connect},
	{"send", nml_send},
	{"shutdown",  nml_shutdown},
	{NULL, NULL}
};
const struct luaL_Reg nml_socket_receive_only [] = {
	{"close", nml_close},
	{"bind", nml_bind},
	{"connect", nml_connect},
	{"receive", nml_recv},
	{"shutdown",  nml_shutdown},
	{NULL, NULL}
};


//uses nml_socket_methods
//assumes that self (socket ud) is in first postion
//table is in second (optional)
//ignores properties that it does not find a handler for.
int nml_bus (lua_State *L) 
{
	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_std, 0); // size of this table will change. don't use newlib
	luaL_newlib(L, nml_options_std_set);
	luaL_newlib(L, nml_options_std_get);
	
 	return nml_generic_socket_constructor(L, NN_BUS, "bus");
}

int nml_pair (lua_State *L) 
{
	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_std, 0); // size of this table will change. don't use newlib
	luaL_newlib(L, nml_options_std_set);
	luaL_newlib(L, nml_options_std_get);
 	return nml_generic_socket_constructor(L, NN_PAIR,"pair");
}

int nml_pub (lua_State *L) 
{
	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_std, 0); // size of this table will change. don't use newlib
	luaL_newlib(L, nml_options_std_set);
	luaL_newlib(L, nml_options_std_get);
 	return nml_generic_socket_constructor(L, NN_PUB, "pub");
}

int nml_sub (lua_State *L) 
{
	int ret;

	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_receive_only, 0); // size of this table will change. don't use newlib

	lua_newtable(L);//subscriptions table
	lua_newtable(L);//metatable
	lua_newtable(L);//upvale is the hidden table that contains the actual subscriptions.
	
	luaL_setfuncs(L, nml_subscriptions_mt, 1); //one upvalue

	lua_setmetatable(L, -2); //apply metatable
	//dump_stack(L, "applied the metatable.");
	lua_setfield(L, -2, "__subscriptions"); //the table that will hold the subscriptions.
	lua_newtable(L);
	luaL_setfuncs(L, nml_options_std_set, 0);
	luaL_setfuncs(L, nml_sub_options_set, 0);
	lua_newtable(L);
	luaL_setfuncs(L, nml_options_std_get, 0);
	luaL_setfuncs(L, nml_sub_options_get, 0);

	ret  =  nml_generic_socket_constructor(L, NN_SUB, "sub");

	lua_getuservalue(L, 1);
	lua_getfield(L, -1, "__subscriptions");
	lua_getmetatable(L, -1);
	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "__socket");
	
	lua_settop(L, ret);


	return ret;
}

int nml_push (lua_State *L) 
{
	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_send_only, 0); // size of this table will change. don't use newlib
	luaL_newlib(L, nml_options_std_set);
	luaL_newlib(L, nml_options_std_get);
 	return nml_generic_socket_constructor(L, NN_PUSH, "push" );
}

int nml_pull (lua_State *L) 
{
	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_receive_only, 0); // size of this table will change. don't use newlib
	luaL_newlib(L, nml_options_std_set);
	luaL_newlib(L, nml_options_std_get);
 	return nml_generic_socket_constructor(L, NN_PULL, "pull");
}
int nml_req (lua_State *L) 
{
	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_std, 0); // size of this table will change. don't use newlib
	lua_newtable(L);
	luaL_setfuncs(L, nml_options_std_set, 0);
	luaL_setfuncs(L, nml_req_options_set, 0);
	lua_newtable(L);
	luaL_setfuncs(L, nml_options_std_get, 0);
	luaL_setfuncs(L, nml_req_options_get, 0);
 	return nml_generic_socket_constructor(L, NN_REQ, "req");
}

int nml_rep (lua_State *L) 
{
	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_std, 0); // size of this table will change. don't use newlib
	luaL_newlib(L, nml_options_std_set);
	luaL_newlib(L, nml_options_std_get);
 	return nml_generic_socket_constructor(L, NN_REP, "rep");
}
int nml_surveyor (lua_State *L) 
{
	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_std, 0); // size of this table will change. don't use newlib
	lua_newtable(L);
	luaL_setfuncs(L, nml_options_std_set, 0);
	luaL_setfuncs(L, nml_surveyor_options_set, 0);
	lua_newtable(L);
	luaL_setfuncs(L, nml_options_std_get, 0);
	luaL_setfuncs(L, nml_surveyor_options_get, 0);
 	return nml_generic_socket_constructor(L, NN_SURVEYOR, "surveyor");
}

int nml_respondent (lua_State *L) 
{
	lua_newtable(L);
	luaL_setfuncs(L, nml_socket_std, 0); // size of this table will change. don't use newlib
	luaL_newlib(L, nml_options_std_set);
	luaL_newlib(L, nml_options_std_get);
 	return nml_generic_socket_constructor(L, NN_RESPONDENT, "respondent");
}