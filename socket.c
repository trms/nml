#include "nml.h"


#define nml_opt_err(L) luaL_error(L,"cannot access option on closed socket.")

typedef enum {NML_MILLIS, NML_BYTES, NML_PRIORITY, NML_DOMAIN, NML_PROTOCOL, NML_CHOICE} nml_socket_property_type;

typedef union  {
	int millis;
	int bytes;
	int priority;
	int domain;
	int protocol;
	int choice;
} nml_socket_property_units;

typedef struct nml_socket_property_t {
	int nn_level;
	int nn_property_name;
	nml_socket_property_units property_value;
	nml_socket_property_type type;
	int (* set_lua_property) (lua_State *, struct nml_socket_property_t *);
	int (* get_lua_property)(lua_State *, struct nml_socket_property_t *);
	nml_socket_property_units property_default;
	
} nml_socket_property_t;

static int nml_connect(lua_State *);
static int nml_bind(lua_State *);
static int nml_send(lua_State *);
static int nml_recv(lua_State *);
static int nml_close(lua_State *);
static int nml_newindex(lua_State *);
static int nml_index(lua_State * );
static int nml_socket_gc(lua_State * );
static int nml_shutdown(lua_State *);

const struct luaL_Reg nml_socket_methods [] = {
	{"__index", nml_index},
	{"__newindex", nml_newindex},
	{"__gc", nml_socket_gc},
	{"close", nml_close},
	{"bind", nml_bind},
	{"connect", nml_connect},
	{"receive", nml_recv},
	{"send", nml_send},
	{"shutdown",  nml_shutdown},
	{NULL, NULL}
};



static int nml_freemsg(lua_State *, nml_socket_ud * self);
int nml_set_milli_prop_0_true (lua_State *, nml_socket_property_t *);
int nml_get_milli_prop_0_true (lua_State * , nml_socket_property_t *);
int nml_set_buffer (lua_State * , nml_socket_property_t *);
int nml_get_buffer (lua_State * , nml_socket_property_t *);
int nml_set_priority (lua_State * , nml_socket_property_t *);
int nml_get_priority (lua_State * , nml_socket_property_t *);
int nml_set_choice (lua_State * , nml_socket_property_t *);
int nml_get_choice (lua_State * , nml_socket_property_t *);
int nml_get_domain (lua_State * , nml_socket_property_t *);
int nml_get_protocol (lua_State * , nml_socket_property_t *);
int nml_set_read_only (lua_State * , nml_socket_property_t *);


//index to match NML_SOCKET_PROPERTY that shadows the NN equivelent.
struct nml_socket_property_t nml_socket_properties_t []  = {
	{NN_SOL_SOCKET, NN_LINGER, 0, NML_MILLIS, nml_set_milli_prop_0_true, nml_get_milli_prop_0_true, NML_DEFAULT_LINGER},
	{NN_SOL_SOCKET, NN_SNDBUF, 0, NML_BYTES, nml_set_buffer, nml_get_buffer},
	{NN_SOL_SOCKET, NN_RCVBUF, 0, NML_BYTES, nml_set_buffer, nml_get_buffer},
	{NN_SOL_SOCKET, NN_SNDTIMEO, 0, NML_MILLIS, nml_set_milli_prop_0_true, nml_get_milli_prop_0_true,  NML_DEFAULT_SNDTIMEO},
	{NN_SOL_SOCKET, NN_RCVTIMEO,0, NML_MILLIS, nml_set_milli_prop_0_true, nml_get_milli_prop_0_true, NML_DEFAULT_RCVTIMEO},
	{NN_SOL_SOCKET,NN_RECONNECT_IVL, 0, NML_MILLIS, nml_set_milli_prop_0_true, nml_get_milli_prop_0_true,NML_DEFAULT_RECONNECT_IVL},
	{NN_SOL_SOCKET, NN_RECONNECT_IVL_MAX, 0, NML_MILLIS, nml_set_milli_prop_0_true, nml_get_milli_prop_0_true,NML_DEFAULT_RECONNECT_IVL_MAX},
	{NN_SOL_SOCKET, NN_SNDPRIO,0, NML_PRIORITY, nml_set_priority, nml_get_priority, NML_DEFAULT_SNDPRIO},
	{NN_SOL_SOCKET, NN_IPV4ONLY, 0, NML_CHOICE, nml_set_priority, nml_get_priority,  NML_DEFUALT_IPV4_ONLY},
	{NN_SOL_SOCKET, NN_DOMAIN,  0, NML_DOMAIN, nml_set_read_only, nml_get_domain,  0},
	{NN_SOL_SOCKET, NN_PROTOCOL, 0, NML_PROTOCOL, nml_set_read_only, nml_get_protocol,  0},
	// {NN_SOL_SOCKET, NN_SNDFD, NULL, sizeof(NML_FD_TYPE)},
	// {NN_SOL_SOCKET, NN_RCVFD, NULL, sizeof(NML_FD_TYPE)},
};


const int NML_NN_OPTIONS[] = {
	NN_LINGER, NN_SNDBUF, NN_RCVBUF, NN_SNDTIMEO, NN_RCVTIMEO, NN_RECONNECT_IVL,
	NN_RECONNECT_IVL_MAX, NN_SNDPRIO, 
	NN_IPV4ONLY, NN_DOMAIN, NN_PROTOCOL, NN_SNDFD, NN_RCVFD,
};

	

const char   NML_SOCKET_OPTION_NAMES[] [32] = {
	"linger",
	"send_buffer",
	"receive_buffer",
	"send_timeout",
	"receive_timeout",
	"reconnect_interval",
	"reconnect_interval_max",
	"send_priority",
	"send_handle",
	"receive_handle",
	"domain",
	"protocol",
	"ipv_4_only",
};

int NML_SOCKET_OPTION_NAMES_COUNT = sizeof(NML_SOCKET_OPTION_NAMES) / (sizeof( * NML_SOCKET_OPTION_NAMES));


//assumes index is at position 2
//helper function for metamethods
nml_socket_property_t * nml_get_method(lua_State * L) {
	int i;
	const char * field_name = lua_tostring(L, 2);

	for (i=0; i < NML_SOCKET_OPTION_NAMES_COUNT; i++) {
		if (strcmp(field_name, NML_SOCKET_OPTION_NAMES[i]) == 0) {
			return  &nml_socket_properties_t[i];
		}
	}
	return NULL;
}


//metemethods for socket object
int nml_index(lua_State * L) {
	nml_socket_ud * nml_s = get_socket(L);
	nml_socket_property_t * prop =  nml_get_method(L);
	size_t ps;
	if(prop){
		ps = sizeof(prop->property_value);
		if (nn_getsockopt(
			nml_s->socket, prop->nn_level, 
			prop->nn_property_name, &prop->property_value, 
			&ps) != 0) {
				//nml_error(L, "error getting socket option");
				lua_pushnil(L);
				return 1;
			}

		return prop->get_lua_property(L, prop);

	}
	else{
		//get this userdata's metatable and return the value
		//at the field_name, or nil.
		lua_getmetatable(L, 1);
		lua_getfield(L, -1, lua_tostring(L,2));
		if (lua_isnil(L, -1)) {
			lua_getuservalue(L, 1);
			lua_getfield(L, -1, lua_tostring(L,2));
		}
		
		return 1;
	}

}
int nml_newindex(lua_State *L) {
	nml_socket_ud * nml_s = get_socket(L);

	nml_socket_property_t * prop =  nml_get_method(L);

	if(prop){ // if we found it...
		prop->set_lua_property(L, prop);

		if (nn_setsockopt(
			nml_s->socket, prop->nn_level, 
			prop->nn_property_name, &prop->property_value, 
			sizeof(prop->property_value)) != 0)
			nml_error(L, "error setting socket option");

		return 0;
		//args are udata, value, boolean.
	}
	return 0;
}
int nml_socket_gc(lua_State *L) {
	//printf("Inside of GC!!\n\n");

	return nml_close(L);
	//return 0;
}

///methods for sockets:
static int nml_bind(lua_State *L) {
	nml_socket_ud * self =  get_socket(L);

	const char * addr_string =luaL_checkstring(L, 2);

	int eid = nn_bind(self->socket, addr_string);
	if (eid != -1){
		lua_getuservalue(L, 1);

		lua_getfield(L, -1, "end_points");

		lua_pushinteger(L, eid);
		lua_newtable(L);
		lua_pushinteger(L, eid);
		//fields of the eid table.
		lua_setfield(L, -2, "id");
		lua_pushvalue(L, 2); //second argument is the url
		lua_setfield(L, -2, "url");
		lua_pushstring(L, "local");
		lua_setfield(L, -2, "end_point_type");

		lua_settable(L, -3); //set the eid table.
	}else if (nn_errno() == ETERM) {
		return 0;
	}
	else
	{
		return nml_error(L, "error when binding");
	}
	lua_settop(L, 1); //return self
	return 1;

}
static int nml_connect(lua_State *L) {
	nml_socket_ud * self =  get_socket(L);

	const char * addr_string =luaL_checkstring(L, 2);

	int eid = nn_connect(self->socket, addr_string);
	if (eid != -1){
		lua_getuservalue(L, 1);

		lua_getfield(L, -1, "end_points");

		lua_pushinteger(L, eid);
		lua_newtable(L);
		lua_pushinteger(L, eid);
		//fields of the eid table.
		lua_setfield(L, -2, "id");
		lua_pushvalue(L, 2); //second argument is the url
		lua_setfield(L, -2, "url");
		lua_pushstring(L, "remote");
		lua_setfield(L, -2, "end_point_type");

		lua_settable(L, -3); //set the eid table.
	}else if (nn_errno() == ETERM) {
		return 0;
	}
	else
	{
		return nml_error(L, "error when connecting");
	}
	lua_settop(L, 1); //return self
	return 1;

}

static int nml_send(lua_State * L) {
	size_t len;
	int sent_bytes;
	int flags = 0;
	nml_socket_ud * self =  get_socket(L);

	self->buf = (char *) lua_tolstring(L, 2, &(len));
	self->len = (long) len;
	
	if(!self->len)
		luaL_error(L, "No message received. Expected string");

	self->len++; //include \0
	
	

	if (lua_toboolean(L, 3)){
		flags = flags | NN_DONTWAIT;
	}

	sent_bytes = nn_send(self->socket, self->buf, self->len, flags);
	if ( sent_bytes < 0 && (NN_DONTWAIT & flags) && (nn_errno() == EAGAIN)  ) {
		lua_pushboolean(L, 1);
		return 1; //true means non-blocking call and nothing is here. 
	}
	else if (sent_bytes < 0 ) {
		return nml_error(L, "error receiving message");

	}
	else { //message sent.
		lua_settop(L, 1);
		//WARINING:::: THIS MAY NEVER GET CALLED, if lua_pushlstring fails.
		//This is a problem, so long as the GC function has not been written.

		self->buf = NULL;
		self->len = 0;
		return 1; // return self
	}
}
static int nml_recv(lua_State * L) {

	nml_socket_ud * self =  get_socket(L);

	int flags = 0;

	if (lua_toboolean(L, 2)){
		flags = flags | NN_DONTWAIT;
	}
	//luaL_error("break");
	self->len = nn_recv(self->socket, &(self->buf),NN_MSG, flags);

	if ( self->len < 0 && (NN_DONTWAIT & flags) && (nn_errno() == EAGAIN)  ) {
		lua_pushboolean(L, 1);
		return 1; //true means non-blocking call and nothing is here. 
	}else if (self->len < 0 && (nn_errno() == ETERM)) {
		return 0;
	}
	else if (self->len < 0 ) {
		return nml_error(L, "error receiving message");

	}
	else { //message received.
		//turn into a string and pass it back, for now. :)
		
		lua_pushlstring(L, (const char *) self->buf, self->len - 1);
		//WARINING:::: THIS MAY NEVER GET CALLED, if lua_pushlstring fails.
		//This is a problem, so long as the GC function has not been written.
		nml_freemsg(L, self);
		self->len = -1;

		
		return 1;
	}
}

//shutdown: shuts down an endpoint.
static int nml_shutdown(lua_State *L) {
	int eid;
	nml_socket_ud * self =  get_socket(L);

	if (lua_istable(L, 2)){
		lua_getfield(L, 2, "id");
		lua_remove(L, 2);
	} 
	//if true was passed in...
	else if (lua_isboolean(L, 2) && lua_toboolean(L, 2)){
		lua_getuservalue(L,1);
		lua_getfield(L, -1, "end_points");
		lua_pushnil(L);
		//dump_stack(L, "before while loop in shutdown.");
		while (lua_next(L, -2) != 0) {
			lua_pop(L, 1);
			eid = lua_tointeger(L, -1);
			if(nn_shutdown(self->socket, eid) == 0 ) {
				lua_pushnil(L);
				
				lua_settable(L, -3);
				lua_pushnil(L);
			}else if (nn_errno() == EINTR) { //inturrupted. need to try again later.
				lua_pushboolean(L, 0);
				lua_pushfstring(L, "Inturrupted, try again. End point is %d still open.", eid);
				return 2;
			}else if (nn_errno() == ETERM) {
				return 0;
			}
			else{
				return luaL_error(L, "Error removing endpoint.");
			}
		}
		lua_settop(L,1);

		return 1;
	}
	eid = luaL_checkinteger(L, 2);

	lua_getuservalue(L,1);

	lua_getfield(L, -1, "end_points");
	lua_remove(L, -2);
	lua_insert(L, 2);

	lua_gettable(L,2);

	if(!lua_istable(L,-1))
		return luaL_error(L,"The ID '%d' for the endpoint does not exist.", eid);

	lua_pop(L,1); //we just wanted to see if it was there.
	if(nn_shutdown(self->socket, eid) == 0 ) {
		//let's get rid of the eid table.
		lua_pushinteger(L, eid);
		lua_pushnil(L);
		lua_settable(L,2);
		lua_pop(L,1); //pop the uservalue
		return 1; //return self
	}else if (nn_errno() == EINTR) { //inturrupted. need to try again later.
		lua_pushboolean(L, 0);
		lua_pushfstring(L, "Inturrupted, try again. end point %d still open.", eid);
		return 2;
	} else if (nn_errno() == ETERM) {
		return 0;
	}
	else
		return nml_error(L, "Error shutting down");
	
}
//closes a socket
static int nml_close(lua_State *L) 
{
	nml_socket_ud * self =  get_socket(L);


	//call the shutdown function
	lua_getfield(L, 1, "shutdown");
	lua_pushboolean(L, 1);
	
	//dump_stack(L,"before calling shutdown");
	lua_call(L, 1, LUA_MULTRET);
	
	//dump_stack(L,"after calling shutdown");
	nml_freemsg(L, self);

	if (nn_close(self->socket)) {
		if (nn_errno() == EINTR) { //inturrupted. need to try again later.
			lua_pushboolean(L, 0);
			lua_pushstring(L, "inturrupted, try again. socket still open.");
			return 2;
		}else if (nn_errno() == ETERM) { //shutting down library
			return 0;
		}
		else //real error happened.
			return nml_error(L, "could not close the socket");
	}
	else {
		
		lua_pushboolean(L, 1);
		return 1;
	}
}

static int nml_freemsg(lua_State * L, nml_socket_ud * self) {
	//TODO: When MSG is an object, I need to
			//abstract the nn_freemsg method.
	if (self->len <= 0)
		return 0;


	if(nn_freemsg(self->buf) < 0)
		return nml_error(L,"API error");
	
	self->len = -1;

	return 1;

}




/////Below are the property getters/setters

int nml_set_milli_prop_0_true (lua_State * L, nml_socket_property_t * prop) {
	nml_socket_property_units  value;

	if (!lua_toboolean(L,-1)){ 
		value  = prop->property_default; //default value.
	} else if (lua_isboolean(L,-1)){ // if it is a boolean, then it must be true here.
		value.millis  = -1; //infinity.
	} else {
		value.millis  = luaL_checkint(L,-1);
	}
	prop->property_value =  value;

	return 0;
}
int nml_get_milli_prop_0_true (lua_State * L, nml_socket_property_t * prop) {
	nml_socket_property_units millis =  prop->property_value;

	if ( *(int *) &millis < 0) //negative one is infinity, and that was passed to us as true, so...
		lua_pushboolean(L, 1);
	else
		lua_pushnumber(L,  *(int *) &millis);

	return 1;
}
int nml_set_buffer (lua_State * L, nml_socket_property_t * prop) {
	nml_socket_property_units  value;
	if (!lua_toboolean(L,-1)){
		value = prop->property_default; 
	} else {
		value.bytes = luaL_checkint(L,-1);
	}
	prop->property_value =  value;
	return 0;
}
int nml_get_buffer (lua_State * L, nml_socket_property_t * prop) {
	lua_pushnumber(L,  *(int *) &prop->property_value);
	return 1;
}
int nml_set_priority (lua_State * L, nml_socket_property_t * prop) {
	nml_socket_property_units  value;
	if (!lua_toboolean(L,-1)){
		value = prop->property_default; 
	} else {
		value.priority = luaL_checkint(L,-1);
	}
	prop->property_value =  value;
	return 0;
}
int nml_get_priority (lua_State * L, nml_socket_property_t * prop) {
	lua_pushnumber(L,  *(int *) &prop->property_value);
	return 1;
}
int nml_set_choice (lua_State * L, nml_socket_property_t * prop) {
	nml_socket_property_units  value;
	if (lua_isnil(L,-1))
		value = prop->property_default; 
	else if (lua_toboolean(L, -1))
		value.choice = 1;
	else
		value.choice = 0;

	prop->property_value =  value;

	return 0;
}
int nml_get_choice (lua_State * L, nml_socket_property_t * prop) {
	lua_pushboolean(L,  *(int *) &prop->property_value);
	return 1;
}
int nml_get_domain (lua_State * L, nml_socket_property_t * prop) {
	int d = *(int *) &prop->property_value;

	int i;
	for(i = 0; i < (NML_NN_DOMAINS_COUNT); i ++) {
		if (NML_NN_DOMAINS[i] == d) {

			lua_pushstring(L, NML_DOMAIN_STRINGS[i]);
			return 1;
		}
	}

	//TODO: I should throw an API warning here.
	lua_pushnumber(L, d);
	return 1;
}
int nml_get_protocol (lua_State * L, nml_socket_property_t * prop) {
	int p = *(int *) &prop->property_value;

	int i;
	for(i = 0; i < (NML_PROTOCOL_COUNT); i ++) {
		if (NML_NN_PROTOCOLS[i] == p) {

			lua_pushstring(L, NML_PROTOCOL_STRINGS[i]);
			return 1;
		}
	}
	//TODO: I should throw an API warning here.
	lua_pushnumber(L, p);
	return 1;
}
int nml_set_read_only (lua_State * L, nml_socket_property_t * prop) {
	return  luaL_error(L, "Cannot set read-only property in nanomsg.");
}
