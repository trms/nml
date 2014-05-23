/***The main socket object.
@module nml.socket
*/


#include "nml.h"
#include "socket.h"
#include <assert.h>

// windows concurrency runtime
#include <concrt.h>
#include <queue>

namespace NNML
{
	struct SSection
	{
		unsigned __int8* pui8Data;
		unsigned __int32 ui32Size;
	};

	struct SChunk
	{
		unsigned __int32 ui32CkId;
		unsigned __int32 ui32CkSize;
		unsigned __int8 aui8Data[];
	};

	SChunk* NextChunk(SChunk* in_psChunk)
	{
		return (SChunk*)(in_psChunk->aui8Data + in_psChunk->ui32CkSize);
	}
};

static void set_all_props(lua_State *);
static int set_domain (lua_State *);
static int freemsg(lua_State *);
static int shutdown_all(lua_State *);

static int call_for_each_in_field (lua_State *, const char * );
static struct property nml_get_nn_opt(const char * index);
static void nml_set_or_error(lua_State * L, int s, struct property prop, void * v, 
	size_t sz, const char * value);
static void nml_get_or_error(lua_State * L, int s, struct property prop, void * v, 
	size_t * sz);


/*const struct nml_socket_methods; */
const struct luaL_Reg nml_socket_metamethods [] = {
	{"__index", nml_index},
	{"__newindex", nml_newindex},
	{"__gc", nml_socket_gc},

	{NULL, NULL}
};

// constants for options

 struct  property{
	const char * name;
	int nn_level;
	int nn_name;
	const void * default_value;
	size_t default_value_sz;

} ;

const int def_linger = 1000;
const int def_buffer = 128 * 1024;
const int def_timeout = -1;
const int def_rec_int = 100;
const int def_rec_int_max = 0;
const int def_snd_prio = 8;
const int def_ipv4 = 1;
const int def_fd = -1;
const int def_surveyor_deadline = 1000;
const char * def_subscribe_str = "";
const int def_req_resend_ivl = 60000;

const struct property properties [] = {
	{"linger", 					NN_SOL_SOCKET, NN_LINGER, 	&def_linger, sizeof(def_linger )},
	{"send_buffer", 			NN_SOL_SOCKET, NN_SNDBUF, 	&def_buffer, sizeof(def_buffer )},
	{"receive_buffer", 			NN_SOL_SOCKET, NN_RCVBUF, 	&def_buffer, sizeof(def_buffer )},
	{"send_timeout", 			NN_SOL_SOCKET, NN_SNDTIMEO, &def_timeout, sizeof(def_timeout )},
	{"receive_timeout", 		NN_SOL_SOCKET, NN_RCVTIMEO, &def_timeout, sizeof(def_timeout )},
	{"reconnect_interval", 		NN_SOL_SOCKET, NN_RECONNECT_IVL, 	 &def_rec_int, 		sizeof(def_rec_int )},
	{"reconnect_interval_max", 	NN_SOL_SOCKET, NN_RECONNECT_IVL_MAX, &def_rec_int_max, 	sizeof(def_rec_int_max )},
	{"send_priority", 			NN_SOL_SOCKET, NN_SNDPRIO,	&def_snd_prio, 	sizeof(def_snd_prio )},
	{"ipv4_only", 				NN_SOL_SOCKET, NN_IPV4ONLY, &def_ipv4, 		sizeof(def_ipv4 )},
	{"send_fd", 				NN_SOL_SOCKET, NN_SNDFD, 	&def_fd, sizeof(def_fd )},
	{"receive_fd", 				NN_SOL_SOCKET, NN_RCVFD, 	&def_fd, sizeof(def_fd )},
	{"domain", 					NN_SOL_SOCKET, NN_DOMAIN, 	NULL, sizeof(int)},
	{"protocol", 				NN_SOL_SOCKET, NN_PROTOCOL,	NULL, sizeof(int)},
	{"subscribe", 				NN_SUB, 	   NN_SUB_SUBSCRIBE, &def_subscribe_str, sizeof(def_subscribe_str) },
	{"unsubscribe", 			NN_SUB, 	   NN_SUB_UNSUBSCRIBE, &def_subscribe_str, sizeof(def_subscribe_str) },
	{"resend_request_interval",	NN_REQ, NN_REQ_RESEND_IVL, &def_req_resend_ivl, sizeof(def_req_resend_ivl)},
	{"surveyor_deadline", NN_SURVEYOR, NN_SURVEYOR_DEADLINE, &def_surveyor_deadline, sizeof(def_surveyor_deadline) },
	{NULL, 0, 0, NULL, sizeof(NULL)},
};

const int nml_to_nn_domains_ar [] = {AF_SP, AF_SP_RAW, -1};
const char nml_domain_str [][16] = {"full", "raw"};

const int protos_str_ar[] =  {
	NN_PAIR, NN_PUB, NN_SUB, NN_REQ, NN_REP, NN_PUSH, 
	NN_PULL, NN_SURVEYOR, NN_RESPONDENT, NN_BUS, -1
};
const char domain_str_ar [][16] = { 
	"pair", "pub", "sub" , "req", "rep", "push", 
	"pull", "surveyor", "respondent", "bus"
};

/***
A nanomsg socket userdata object.
@type socket 

*/

///The socket domain (raw or full socket) (read only)
//@field[string] domain 



///The protocol that the socket was created with. (read only)
//@field protocol


///The time that a socket tries to send pending outbound messages after close has been called, in milliseconds. A false value (`false` or `nil`) means infinite linger. 
//Default value is `1000` (1 second).
//@field linger 

///Size of the send buffer, in bytes. To prevent blocking for messages larger than the buffer, exactly one message may be buffered in addition to the data in the send buffer. Default value is 128kB.
//@field send_buffer

///Size of the receive buffer, in bytes. To prevent blocking for messages larger than the buffer, exactly one message may be buffered in addition to the data in the receive buffer. Default value is 128kB.
//@field receive_buffer

///The timeout for send operation on the socket, in milliseconds. If message cannot be sent within the specified timeout, `nil` plus a timeout message is returned. False (`false`, but NOT `nil`) means infinite timeout (block forever). Default is false.
//@field send_timeout 

///The timeout for receive operation on the socket, in milliseconds. If no messages are waiting, then `false` is returned. False (`false`, but NOT `nil`) means infinite timeout (block forever). Default is false
//@field receive_timeout 

///For connection-based transports such as TCP, this option specifies how long to wait, in milliseconds, when connection is broken before trying to re-establish it. Note that actual reconnect interval may be randomised to some extent to prevent severe reconnection storms. Default value is `100` (0.1 second).
//@field reconnect_interval 

///This option is to be used only in addition to the @{reconnect_interval} option. It specifies maximum reconnection interval. On each reconnect attempt, the previous interval is doubled until `reconnect\_interval\_max` is reached. Value of `false` means that no exponential backoff is performed and reconnect interval is based only on @{reconnect_interval}. If `reconnect\_interval\_max` is less than @{reconnect_interval}, it is ignored. Default value is `false`.
//@field reconnect_interval_max 

///This is outbound priority of the socket. This option has no effect on socket types that send messages to all the peers. However, if the socket type sends each message to a single peer (or a limited set of peers), peers with high priority take precedence over peers with low priority. Highest priority is 1, lowest priority is 16. Default value is 8.
//@field send_priority

///If set to `true`, only IPv4 addresses are used. If set to `false`, both IPv4 and IPv6 addresses are used. Default value is `true`.
//@field ipv_4_only 



//metemethods for socket object
int nml_index(lua_State * L)
{
	const char * index = lua_tostring(L,2);
	lua_getmetatable(L, 1);
	lua_getfield(L, -1, index); //normal metamethod for class
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1); //pop nil
		lua_getuservalue(L, 1);
		lua_getfield(L, -1, index); // obj method access
		if(lua_isnil(L, -1)) {
			lua_pop(L, 1); //pop nil again
			lua_getfield(L, -1, "nml_get_properties"); //getter table
			lua_getfield(L, -1, index);
			if(!lua_isnil(L, -1)){
				//found the method.
				// dump_stack(L, "found option. calling -2 with one argument.");
				lua_pushvalue(L, 1); //push self to top.
				lua_pushvalue(L, 2); //push key.
				lua_call(L, 2, 1);
			} //else return the nil
		}
	}
	return 1;
}

int nml_newindex(lua_State *L)
{
	lua_getuservalue(L, 1);
	lua_getfield(L,4, "nml_set_properties");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if(!lua_isnil(L, -1)) {
		//dump_stack(L, "setting. removing -2, removing 2, insert 1, call with 2 args.");
		lua_remove(L, -2); //remove the setter table from stack.
		lua_remove(L, -2); //remove uservalue table from stack.
		//leave the key on the stack, in case a function is used
		//for multiple properties.
		lua_insert(L, 1); //put function first.
		lua_settop(L,4);
		//dump_stack(L, "lua_call(L, 3, 0);");
		lua_call(L, 3, 0);
		//dump_stack(L, "after call");
	}
	return 0;
}

int nml_socket_gc(lua_State *L) {
	return nml_close(L);
}

//No matter the pattern, this function is used to create a socket.
int nml_generic_socket_constructor (lua_State *L, int nn_proto, const char * proto_name) 
{
	int ret_count = 2;
	int domain;
	//make a new userdata object with the NML_SOCKET metatable and make it in the
	//first postion, in true selfish fashion.
	//dump_stack(L, "stack: ENTER! opt table or no opts table.");
	nml_socket_ud * self = (nml_socket_ud *) lua_newuserdata(L, sizeof(nml_socket_ud));
	luaL_getmetatable(L, NML_SOCKET);
	lua_setmetatable(L, -2);
	lua_insert(L, 1);

	domain = set_domain(L);
	//makes the socket.
	self->socket = nn_socket(domain, nn_proto);

	if (self->socket >= 0) {

		self->len = 0;
		 // dump_stack(L, "socket_made: self, opt_table?, methods, nml_set_props, nml_get_props");
		lua_setfield(L, -3, "nml_get_properties");
		lua_setfield(L, -2, "nml_set_properties");
//		dump_stack(L, "pair 50 stack: self, opt_table?, uservalue");
		//set the id for the protocol
		// dump_stack(L, "getters/setters set: self, opt_table?, methods/uservalue table (uv)");
		lua_pushboolean(L, 1);
		lua_setfield(L, -2, proto_name);
		lua_pushstring(L, proto_name);
		lua_setfield(L, -2, "protocol");
//		dump_stack(L, "stack: ud, opts, uservalue. id has been set.");
		//end_points table. empty, for now.
		lua_newtable(L);
		lua_setfield(L, -2, "end_points");

//		dump_stack(L, "stack: ud, opts, uservalue. methods set.");
		lua_setuservalue(L, 1);
//		dump_stack(L, "stack: ud, opts. uservalue set.");
		//process any fields provided in the argument table (optional)
		set_all_props(L);
		// dump_stack(L, "stack: ud, opts. props set");
		call_for_each_in_field(L, "bind");
		// dump_stack(L, "stack bind called: ud, opts. props set");
		call_for_each_in_field(L, "connect");
		// dump_stack(L, "stack connect called: ud, opts. connect/bind set.");


	} else {
		ret_count = 2;
		lua_pushnil(L);
		lua_pushfstring(L, "Could not create nanomsg socket: %s\n", nml_errorstr());
	}
	lua_settop(L, ret_count);
	return ret_count;
}
/***
Binds a socet to a local transport
@function bind
@param[type=nml_socket]		self
@param[type=string] address	The address that the socket will bind to. 
@treturn[1] nml_socket Returns self
@return[2] nil
@return[2] error message
*/
int nml_bind(lua_State *L)
{
	int ret_count = 1;
	const char * addr_string;
	nml_socket_ud * self;
	int eid;

	self =  nml_get_socket(L);
	addr_string = luaL_checkstring(L, 2);
	
	eid = nn_bind(self->socket, addr_string);

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
		lua_settop(L, 1); //return self
		lua_pushinteger(L,eid); //also return eid
		ret_count = 2;
	}else if (nn_errno() == ETERM) 
		ret_count = nml_close(L);
	else
		nml_error(L, "error when binding");

	return ret_count;
}

/***
Connects a socket to a remote endpoint.
@function connect
@param[type=nml_socket]		self
@param[type=string] address	The address that the socket will bind to. 
@treturn[1] nml_socket Returns self
@return[2] nil
@return[2] error message
*/
int nml_connect(lua_State *L)
{
	int ret_count = 1;
	const char * addr_string;
	nml_socket_ud * self;
	int eid;
	
	self =  nml_get_socket(L);

	//We're going to error if there is no address string.
	addr_string = luaL_checkstring(L, 2);
	eid = nn_connect(self->socket, addr_string);
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
		lua_settop(L, 1); //return self
		lua_pushinteger(L, eid); //also push the eid number.
		ret_count = 2;
	} else if (nn_errno() == ETERM)
		ret_count = nml_close(L); //closing the library.
	else
		nml_error(L, "error when connecting");
	
	return ret_count;

}
/***
Sends a message to connected end points.
@function send
@param[type=nml_socket]		self
@param[type=string] message	The message to send. 
@param[opt=true] block If true or none or nil, this call will block on send. If false, then it will not *block*, even if the socket was not ready to send. In this case, false is returned.
@treturn[1] nml_socket Returns self
@return[2] nil
@return[2] error message.
*/
int nml_send(lua_State * L)
{
	int ret_count = 1;
	int flags = 0;
	nml_socket_ud * self =  nml_get_socket(L);

	// our goal is to use nanomsg to send a single bucket of data
	// ideally I would send it in multiple parts, but that requires the receiver to know in advance how many parts to expect..
	// the data can be formatted either as a string (serialized as a string) or binary data (table that contains ONLY data+size members)
	// in that last scenario the calling lua code needs to serialize any non-data items prior to calling send (ex: serialize({pos=1}) --> sends the string version of the table)

	/*
	
	local nn_message = 
	{
		[1] = false,
		[2] = {base= ,len= ,},
		[3] = {base= ,len= ,},
		-- control is an opaque type to nml
		-- warning! math the control sequence with the nn_buffer buffer sequence
		control = serialize(
		{
			[1] = {media_type="video", width=,height= ,...},
			[2] = {media_type="video", width=,height= ,...},
		}),
		controllen = string.len(self.control)
	}

	*/

	size_t sizeBuffer;
	unsigned __int8* pui8Buffer;

	// 1. calculate the buffer size
	// if we're getting a string then handle it as an opaque type
	if (lua_isstring(L, 2)) {
		const char* pch = lua_tolstring(L, 2, &sizeBuffer);
		if(!sizeBuffer)
			luaL_error(L, "No message received. Expected string");

		sizeBuffer++; //include \0
		pui8Buffer = (unsigned __int8*)nn_allocmsg(sizeBuffer, 0);

		// copy the data
		memcpy(pui8Buffer, pch, sizeBuffer-1); // no \0
	} else {
		int iSectionsCount;
		std::queue<NNML::SSection> listSections;
		NNML::SSection sControl = {0};

		sizeBuffer = 12; // RIFF, fullsize, NML

		// if we're getting a table it means we need to format it and then send it out as an opaque type
		lua_len(L, 2);
		iSectionsCount = (int)lua_tointeger(L, -1);
		lua_pop(L, 1);

		// calculate the buffer size
		// {base=, len=}
		dump_stack(L, "NML:send");
		for (int i=0; i<iSectionsCount; i++) {
			// [i]={base=, len=}
			lua_pushinteger(L, i+2); // sections start at [2]
			lua_gettable(L, 2);

			if (lua_isnil(L, -1)) {
				// this section doesn't have a base, len. ignore it
				lua_pop(L, 1);
			}
			else {
				// {base=, len=}
				lua_pushstring(L, "base");
				lua_gettable(L, -2);
				if(lua_isnil(L, -1))
					luaL_error(L, "can't find the base field in {{base=, len=}}");
				else {
					lua_pushstring(L, "len");
					lua_gettable(L, -3);
					if(lua_isnil(L, -1))
						luaL_error(L, "can't find the len field in {{base=, len=}}");
					else {
						NNML::SSection sSection = {(unsigned __int8*)lua_touserdata(L, -2), (unsigned __int32)lua_tointeger(L, -1)};
						listSections.push(sSection);
						sizeBuffer += ((int)lua_tointeger(L, -1) + 8); // add the ckid, cksize
					}
				}
				// base, len, {}
				lua_pop(L, 3);
			}
		}
		// get the control data - description for the above data, packed by the caller, treated as an opaque type here
		// {control=, controllen=, {base=, len=}, {base=, len=}...}
		lua_pushstring(L, "control");
		lua_gettable(L, 2);
		sControl.pui8Data = (unsigned __int8*)lua_touserdata(L, -1);
		if (lua_isnil(L, -1))
			luaL_error(L, "can't find the control field");
		else {
			lua_pushstring(L, "controllen");
			lua_gettable(L, -3);

			if (lua_isnil(L, -1))
				luaL_error(L, "can't find the control field");
			else {
				sControl.ui32Size = (unsigned __int32)lua_tointeger(L, -1);
				sizeBuffer += (int)(lua_tointeger(L, -1) + 8);
			}
		}

		// TODO: see if we can recycle a buffer that matches the requirement

		// if not then allocate a new buffer - use nn's allocation mechanism
		pui8Buffer = (unsigned __int8*)nn_allocmsg(sizeBuffer, 0);

		// populate the buffer
		unsigned __int32 ui32Offset = 0;
		unsigned __int32* pui32Buffer = (unsigned __int32*)pui8Buffer;

		// RIFF
		pui32Buffer[ui32Offset++] = NNML::g_fourCCRIFF;
		pui32Buffer[ui32Offset++] = 0; // filled later
		pui32Buffer[ui32Offset++] = NNML::g_fourCCNML; // nml

		// put the control data at the front
		pui32Buffer[ui32Offset++] = NNML::g_fourCControl;
		pui32Buffer[ui32Offset++] = sControl.ui32Size;
		memcpy(&pui32Buffer[ui32Offset], sControl.pui8Data, sControl.ui32Size);
		ui32Offset += (sControl.ui32Size>>2);

		// populate the sections
		for (int i=0; i<iSectionsCount; i++) {
			NNML::SSection sSection = listSections.front();
			listSections.pop();

			pui32Buffer[ui32Offset++] = NNML::g_fourCCSection;
			pui32Buffer[ui32Offset++] = sSection.ui32Size;
			memcpy(&pui32Buffer[ui32Offset], sSection.pui8Data, sSection.ui32Size);
			ui32Offset += (sSection.ui32Size>>2);
		}
		assert(ui32Offset == sizeBuffer);
	}

	//blocking argument third argument is a boolean and false, then set flag to do not wait.
	if (lua_isboolean(L, 3) && ( !lua_toboolean(L, 3) )){
		flags = flags | NN_DONTWAIT;
	}

	if ((flags&NN_DONTWAIT)==0)
		Concurrency::Context::Oversubscribe(true);

	// pass a ** data
	int iResult = nn_send(self->socket, &pui8Buffer, NN_MSG, flags);

	if ((flags&NN_DONTWAIT)==0)
		Concurrency::Context::Oversubscribe(false);

	if ( iResult < 0) {
		if ( nn_errno() == EAGAIN ) {
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Not ready to send. Retry.");
			ret_count = 2;
		} else if (nn_errno() == ETERM) {
			ret_count = nml_close(L); //library terminating, call close.
		} else
			nml_error(L, "error receiving message");
	}
	else { //message sent.
		self->buf = NULL; //lua owns the string and will get rid of it when settop is called
		self->len = 0;
		lua_settop(L, 1); // return self
	}
	return ret_count; 
}

/***
Receives a message to connected end points.
@function receive
@param[type=nml_socket]		self
@param[opt=true] block If true or none or nil, this call will block on receive. If false, then it will not *block*, even if there is no message to receive. In this case, false is returned.
@treturn[1] string Returns the message
@treturn[2] bool Returns false, if the call was not blocking and there wasn't a message ready to receive.
@return[3] nil
@return[3] error message.
*/
int nml_recv(lua_State * L)
{
	//we can't put this directly into size_t, in case it's unsigned.
	int received_bytes;
	int ret_count = 1;
	int flags = 0;
	int opt;
	size_t opt_size;
	nml_socket_ud * self =  nml_get_socket(L);

	//blocking argument second argument is a boolean and false, then set flag to do not wait.
	if (lua_isboolean(L, 2) && ( !lua_toboolean(L, 2) )){
		flags = flags | NN_DONTWAIT;
	}

	if ((flags&NN_DONTWAIT)==0)
		Concurrency::Context::Oversubscribe(true);

	received_bytes = nn_recv(self->socket, &(self->buf),NN_MSG, flags);

	if ((flags&NN_DONTWAIT)==0)
		Concurrency::Context::Oversubscribe(false);

	if ( received_bytes < 0) {
		if (nn_errno() == EAGAIN  )
			lua_pushboolean(L, 0);  //false means non-blocking call and nothing is here.
			//this MAY be a source of bugs, because nil, errmsg means an error.
			//so the user really needs to check type here.
		else if (nn_errno() == ETERM) {
			ret_count = nml_close(L); //library terminating, call close.
		}
		else if(nn_errno() == ETIMEDOUT) {
			lua_pushboolean(L, 0);
			lua_pushstring(L, "TIMEOUT");
			ret_count = 2;
		}
		else if (nn_errno() == EFSM){

			nn_getsockopt(self->socket, NN_SOL_SOCKET, NN_PROTOCOL, &opt, &opt_size);

			if (opt == NN_SURVEYOR){
				///https://github.com/nanomsg/nanomsg/issues/194
				//////WARNING!!!! This is wrong. should be ETIMEDOUT
				//special case for surevey pattern.
				///Look for nanomsg team to turn this back into ETIMEDOUT
				lua_pushboolean(L, 0);
				lua_pushstring(L, "TIMEOUT");
				ret_count = 2;
			} else {
				nml_error(L, "Error receiving message");
			}
		}
		else {
			nml_error(L, "Error receiving message");
		}
	}
	else { //message received.
		/*
		A RIFF id and size, NML type format. 
		A Control chunk.
		Zero, one or many section chunks.

		0    4    8    16   24   32
		|RIFF|SIZE|NML |CTRL|SIZE|DATA_______|SECT|SIZE|DATA______|SECT|SIZE|DATA______|...
		
		*/
		// it starts with a control section
		unsigned __int32* pui32Buffer = (unsigned __int32*)self->buf;
		unsigned __int32 ui32Offset = 0;

		if ((pui32Buffer[0]==NNML::g_fourCCRIFF) && (pui32Buffer[2]==NNML::g_fourCCNML)) {
			// extract the control data - this is an opaque type, probably the result of a serialize() call
			// the caller will know how to deal with it, just pass it along here
			if (pui32Buffer[3]==NNML::g_fourCControl) {
				// {control=data}
				lua_pushstring(L, "control");
				lua_pushlightuserdata(L, &(pui32Buffer[5]));
				lua_settable(L, -3);

				// {controllen=1234}
				lua_pushstring(L, "controllen");
				lua_pushinteger(L, pui32Buffer[4]);
				lua_settable(L, -3);
			}
			// fill the sections
			// {[1]=lud*} -- use this field to store the void* coming from nn_alloc, we'll pass it to nn_free later
			lua_pushinteger(L, 1);
			lua_pushlightuserdata(L, (void*)self->buf);
			lua_settable(L, -3);

			// {[n]={base=, len=}
			int iIndex = 2; // start at [2]
			ui32Offset = pui32Buffer[4] + 5; // point to the next chunk
			while(ui32Offset<self->len) {
				if (pui32Buffer[ui32Offset]==NNML::g_fourCCSection) {
					lua_pushinteger(L, iIndex);
					lua_newtable(L);

					lua_pushstring(L, "base");
					lua_pushlightuserdata(L, &pui32Buffer[ui32Offset+2]);
					lua_settable(L, -3);

					lua_pushstring(L, "len");
					lua_pushinteger(L, pui32Buffer[ui32Offset+1]);
					lua_settable(L, -3);

					lua_settable(L, -3);
				}
			}
			// set the owner tag, this is how the caller will know it has to give us the buffer back so we can free it
			// {alloc={"nml", buffer_ptr_to_free}
			lua_pushstring(L, "alloc");
			lua_newtable(L);
			
			lua_pushinteger(L, 1);
			lua_pushstring(L, "nml");
			lua_settable(L, -3);
			
			lua_pushinteger(L, 2);
			lua_pushlightuserdata(L, pui32Buffer);
			lua_settable(L, -3);

			lua_settable(L, -3);

			// I'm not going to use this mechanism to track the buffer
			self->buf = nullptr;
			self->len = 0;
		} else {
			// not sure what we got here, use the old way (string)

			//we set len here because that's how freemsg knows that there
			//is a value to free.
			self->len = received_bytes;
			//		printf("the number of bytes received is %lu\n\n", self->len);
			//lua does not want the \0 when it receives a length.
			lua_pushlstring(L, (const char *) self->buf, received_bytes - 1);
			//WARINING:::: THIS MAY NEVER GET CALLED, if lua_pushlstring fails.
			//GC should get it, however.
			freemsg(L);
		}
	}
	return ret_count;
}

/***
Shuts down a socket's endpoint.
@function shutdown
@param[type=nml_socket]		self
@tparam bool|number|table end_point If boolean `true`, all endpoints are removed. If a number or the `socket.end_points[indx]` table, then the specified endpoint is removed.
@treturn[1] nml_socket self
@return[2] nil
@treturn[2] string  error message. This is usually caused by an invalid end_point.
*/
int nml_shutdown(lua_State *L)
{
	int eid;
	int ret_count = 1;
	nml_socket_ud * self;

	self =  nml_get_socket(L);
	lua_settop(L, 2); //let there be self, eid/true/eid table
	//dump_stack(L, "Enter nml_shutdown");
	if (lua_isboolean(L, 2) && lua_toboolean(L, 2)){
		//printf("\n\nisboolean and tobolean\n\n");
		//if true was passed in...
		//call shutdown all.
		//dump_stack(L, "Calling shutdown_all from nml_shutdown");
		return shutdown_all(L);
	}
	
	if (lua_istable(L, 2)){
		//dump_stack(L, "nml_shutdown table in second spot");
		//if they passed in the endpoint object, get the id
		//and remove the table from the stack.
		lua_getfield(L, 2, "id");
		lua_remove(L, 2);
	}
	//we'll error if there is no integer in the second possition.
	//dump_stack(L, "nml_shutdown making integer");
	
	eid = (int) luaL_checkinteger(L, 2);

	//printf("Socket %d is now going to remove endpoint %d\n", self->socket, eid);
	//stack is now self, end_points
	if(nn_shutdown(self->socket, eid) == 0 ) { //if success
		//dump_stack(L, "removing endpoints entry");
		//let's get rid of the eid table.
		lua_getuservalue(L,1);
		lua_getfield(L, -1, "end_points");
		lua_remove(L, -2); //remove the uservalue table
		lua_insert(L, 2); //move end_points to 2. this makes eid at 3
		lua_pushnil(L); // at four

		lua_settable(L, 2);
		lua_pop(L,1); //pop the end_points table.
	}else if (nn_errno() == EINTR) { //inturrupted. need to try again later.
		lua_pushboolean(L, 0);
		lua_pushfstring(L, "Inturrupted, try again. end point %d still open.", eid);
		ret_count =  2;
	} else if (nn_errno() == ETERM) {
		//dump_stack(L, "IN ETERM, WHICH MEANS THAT WE CAN'T SHUTDOWN.");
		ret_count = nml_close(L);
	}
	else
		nml_error(L, "Error shutting down");

	return ret_count;
}
static int getfd(lua_State *L){
	lua_getfield(L, 1, "fd");
	return 1;
}
int nml_getfd (lua_State * L) 
{
	int ret_count;
	int  value;
	size_t val_size;
	nml_socket_ud * self;
	struct property prop;

	ret_count = 1;
	val_size = sizeof(value);
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	nml_get_or_error(L, self->socket, prop, 
		(void *) &value, &val_size);

	lua_newtable(L);
	lua_pushinteger(L, value);
	lua_setfield(L, -2, "fd");
	lua_pushcfunction(L, getfd);
	lua_setfield(L, -2, "getfd");
	//dump_stack(L, "ending nml_getfd");

	return ret_count;
}

/***
Closes a socket.
@function close
@param[type=nml_socket]		self
@treturn[1] nml_socket self
@return[2] nil
@return[2] error message.
*/
int nml_close(lua_State *L) 
{
	int ret_count = 1;
	
	nml_socket_ud * self =  nml_get_socket(L);
	
	//dump_stack(L, "nml_close top with socket");


	// if the library is not terminating, then call shutdown
	// WARNING: I don't check to see if EINTR was thrown 
	// during shutdown.
	/*
	if (nn_errno() != ETERM){
		printf("IN close in != ETERM !!!\n\n\n");
			//luaL_error(L, "break");
		lua_getfield(L, 1, "shutdown");
		lua_pushvalue(L,1);
		lua_pushboolean(L, 1);

		lua_call(L, 2, LUA_MULTRET);

	}
	*/

	//nml_free will check to see if there is a message to be freed and then free it.
	freemsg(L);

	 
	if ( nn_close(self->socket) != 0 ) {
		if (nn_errno() == EINTR) { //inturrupted. need to try again later.
			lua_pushboolean(L, 0);
			lua_pushstring(L, "Inturrupted when closing the socket. Socket still open.");
			ret_count =  2;
		}
		else //real error happened.
			nml_error(L, "Error when closing the socket");
	}
	
	//printf("At end of nml_close. ret_count %d\n\n\n", ret_count);
	return ret_count;
}


//////////property getters and setters


//nil/false = default, true is forever, int is millis
int nml_set_linger (lua_State * L)
{
	int value;
	int ret_count = 0;
	struct property prop;
	nml_socket_ud * self; 
	
	
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	if (lua_isboolean(L, 3) || lua_isnil(L, 3)){
		if(lua_toboolean(L, 3))
			value = -1;
		else //false / nil is default linger
			value = *(int *) prop.default_value;
	} else{
		value = (int) luaL_checkint(L, 3);
	}
	nml_set_or_error(
		L, self->socket, prop, &value, sizeof(value), lua_tostring(L, 3));
	
	return ret_count;
}

int nml_get_linger (lua_State * L)
{
	int  value;
	size_t val_size;
	int ret_count;
	nml_socket_ud * self;
	struct property prop;
	
	val_size = sizeof(value);
	ret_count = 1;
	self =  nml_get_socket(L);

	
	
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	nml_get_or_error(L, self->socket, prop,
		(void *) &value, &val_size);

	if (value < 0)
		lua_pushboolean(L, 1); //lingers forever is true
	else
		lua_pushinteger(L, value);

	return ret_count;


}

//set/get any int and use nanomsg error handling for bad values or illegal access (socket in a bad state)
int nml_set_int (lua_State * L)
{
	int  value;
	int ret_count = 0;
	nml_socket_ud * self;
	struct property prop;

	self=  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	value = lua_isnil(L, 3) ? (* (int *) prop.default_value ): (int) luaL_checkint(L, 3);

	nml_set_or_error(L, self->socket,prop, &value, sizeof(value),lua_tostring(L, 3));

	return ret_count;
}
int nml_get_int (lua_State * L)
{
	int ret_count = 1;
	int  value;
	size_t val_size;

	nml_socket_ud * self;
	struct property prop;

	val_size = sizeof(value);
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	nml_get_or_error(L, self->socket, prop, 
		(void *) &value, &val_size);

	lua_pushinteger(L, value);

	return ret_count;
}
//nil = default (true) truthy values true, falsy is only false
int nml_set_ipv4only (lua_State * L)
{
	int  value;
	int ret_count;
	nml_socket_ud * self; 
	struct property prop ;

	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));
	ret_count = 0;
	if (lua_isnil(L, 3))
		value = *(int *) prop.default_value;
	else
		value = lua_toboolean(L, 3);

	nml_set_or_error(L, self->socket,prop, &value, sizeof(value),lua_tostring(L, 3));
	return ret_count;
}
int nml_get_ipv4only (lua_State * L)
{
	int ret_count; 
	int  value;
	size_t val_size; 
	nml_socket_ud * self; 
	struct property prop; 

	ret_count = 1;
	val_size = sizeof(value);
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	nml_get_or_error(L, self->socket, prop, 
		(void *) &value, &val_size);

	lua_pushboolean(L, value);

	return ret_count;
}
//errors on true, which might be weird
//traps negative because I'm trying to guard against unintential setting on one hand and
//intentional infinate on the other.
//false is infinate timeout (forever block)
//nil = default
//int is millis.
int nml_set_timeout (lua_State * L)
{
	int  value;
	int ret_count; 
	nml_socket_ud * self; 
	struct property prop;
	ret_count = 0;
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));
	//check value
	if (lua_isnil(L, 3))
		value = *(int*) prop.default_value;
	else if (lua_isboolean(L, 3) ) {
		if(!lua_toboolean(L, 3)) 
			value = -1;
		else
			luaL_error(L,"boolean true does not make sense as a time out value.");
	}else{
		value = (int) luaL_checkint(L, 3);
		if(value < 0 ) {
			luaL_error(L, "illegal value to set timeout. Don't use negative values. Use false, instead.");
		}
	}

	nml_set_or_error(L, self->socket, prop, &value, sizeof(value), lua_tostring(L, 3));

	return ret_count;
}
int nml_get_timeout (lua_State * L)
{
	int  value;
	size_t val_size;
	int ret_count; 
	nml_socket_ud * self; 
	struct property prop; 
	
	val_size = sizeof(value);
	ret_count = 1;
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));
	
	nml_get_or_error(L, self->socket, prop, (void *) &value, &val_size);

	(value < 0) ? lua_pushboolean(L, 0) : lua_pushinteger(L, value);


	return ret_count;


}
//nil = default (false)
//false is no backoff.
//-value nn_reconnection is ignored.
//+val is millis
int nml_set_reconnect_interval_max (lua_State * L)
{
	int  value;
	int ret_count; 
	nml_socket_ud * self; 
	struct property prop;

	ret_count = 0;
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	if (lua_isnil(L, 3)){
		value = *(int*) prop.default_value;
	} else if (lua_isboolean(L,3) && !lua_toboolean(L,3)) 
	{
		value = 0;
	}
	else{
		value = (int) luaL_checkint(L, 3);
	}

	nml_set_or_error(
		L, self->socket, prop, &value, sizeof(value),
		lua_tostring(L, 3));

	return ret_count;
}
int nml_get_reconnect_interval_max (lua_State * L)
{
	int  value;
	size_t val_size;
	int ret_count; 
	nml_socket_ud * self; 
	struct property prop; 

	val_size = sizeof(value);
	ret_count = 1;
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	nml_get_or_error(L, self->socket, prop, ( void * ) &value, &val_size);
	
	(value == 0 ) ? lua_pushboolean(L, 0) : lua_pushinteger(L, value);
	
	return ret_count;
}
int nml_set_read_only (lua_State * L)
{
	return luaL_error(L, "Attempt to set '%s', a read-only property.", luaL_checkstring(L, 2));
}
int nml_get_domain (lua_State * L)
{
	
	int  value;
	int i;
	size_t val_size; 
	int ret_count; 
	nml_socket_ud * self; 
	struct property prop; 

	val_size = sizeof(value);
	ret_count = 1;
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	nml_get_or_error(L, self->socket, prop, (void *) &value, &val_size);

	for(i = 0; nml_to_nn_domains_ar[i] != -1; i++) {
		if (nml_to_nn_domains_ar[i] == value) {
			lua_pushstring(L, nml_domain_str[i]);
			break;
		}
	}
	if (nml_to_nn_domains_ar[i] == -1) luaL_error(L, "API error. Unknown socket domain.");

	return ret_count;


}
int nml_get_protocol (lua_State * L)
{
	
	int  value;
	int i;
	size_t val_size; 
	int ret_count; 
	nml_socket_ud * self; 
	struct property prop; 

	val_size = sizeof(value);
	ret_count = 1;
	self =  nml_get_socket(L);
	prop = nml_get_nn_opt(luaL_checkstring(L, 2));

	nml_get_or_error(L, self->socket, prop, (void *) &value, &val_size);

	for(i = 0; protos_str_ar[i] != -1; i++) {
		if (protos_str_ar[i] == value) {
			lua_pushstring(L, domain_str_ar[i]);
			break;
		}
	}
	if (protos_str_ar[i] == -1) luaL_error(L, "API error. Unknown socket protocol.");

	return ret_count;


}
int nml_subscribe__len (lua_State * L) 
{
	int count;

	lua_pushvalue(L, lua_upvalueindex(1));
	lua_pushnil(L);
	for(count = 0; lua_next(L,lua_upvalueindex(1)); ++count)
		lua_pop(L,1);
	lua_pushnumber(L, (lua_Number ) count);
	return 1;
}

int subscribe_next (lua_State *L) {

	// dump_stack(L, "in subscribe_next");
	while(lua_next(L, -2)) {
		lua_pop(L, 1); //numeric key, pop value.
		lua_pushboolean(L, 1);
		// dump_stack(L, "returning from subscribe_next with value");
		return 2;
	}
	lua_pushnil(L); lua_pushnil(L);
	// dump_stack(L, "returning from subscribe_next: end of loop");
	return 2;
}
int nml_subscribe__pairs (lua_State *L)
{
	// dump_stack(L, "inside pairs");
	lua_pushcfunction(L, subscribe_next);
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_pushnil(L);
	return 3;
}
int nml_subscribe__index (lua_State * L) 
{
	int ret_count = 1;
	
	// dump_stack(L, "nml_subscribe__index called");
	lua_gettable(L,lua_upvalueindex(1));
	 // dump_stack(L, "end nml_subscribe__index");

	return ret_count;
}
int do_sub_newindex(lua_State * L) 
{

	//actually, change this to be a table.
	
	char buf [NML_SUB_STRING_MAX];
	size_t str_sz;
	struct property prop;
	int ret_count;
	int exists;
	int set_nn; 
	nml_socket_ud * self; 

	ret_count = 0;
	set_nn = 0;

	self =  nml_get_socket(L);
	// dump_stack(L, "__subscriptions.__newindex\nstack is: userdata, subscribe (table), key (string name of subscription), boolean (add or don't)");

	//already exists or doesn't?
	lua_pushvalue(L, -2);
	lua_gettable(L,lua_upvalueindex(1));
	
	exists = !lua_isnil(L,-1);
	lua_pop(L,1); //we're trying to keep the stack tight. Remove the value.
	if (exists ) {
		if (lua_toboolean(L, -1)) { //replacing value, but who cares? 
			// don't store their value for now. it's always the index of the reverse lookup.
			//cleanup stack and bail.
			lua_settop(L, 1);
			return 0;
		}else { //erasing
			//prepare call.
			// dump_stack(L, "erasing");
			lua_pop(L,1); //pop the value, in case it was false. we don't do false, only nil
			lua_pushvalue(L, -1); //copy the key
			lua_pushnil(L);
			lua_settable(L,  lua_upvalueindex(1));
			//everything is now erased.

			// dump_stack(L, "erasing: prepare call");

			prop = nml_get_nn_opt("unsubscribe");
			str_sz = luaL_len(L,-1) + 1;
			strncpy_s(buf, NML_SUB_STRING_MAX, (const char *) luaL_checkstring(L, -1), NML_SUB_STRING_MAX);
			set_nn = 1;
		}
	} else { // not already there.
		if (!lua_toboolean(L, -1)){ //erasing something that's not there.
			lua_settop(L, 1);
			return 0;
		} else { //adding something not there.
			// dump_stack(L, "adding something");
			lua_pop(L, 1); //don't care what value is, for now.
			lua_pushvalue(L, -1);//one copy.
			lua_pushboolean(L, 1);
			lua_settable(L, lua_upvalueindex(1)); //set the string key to the index of the reverse lookup.
			
			//prepare call.
			prop = nml_get_nn_opt("subscribe");
			str_sz = luaL_len(L,-1);
			strncpy_s(buf, NML_SUB_STRING_MAX, (const char *) luaL_checkstring(L, -1), NML_SUB_STRING_MAX);
			str_sz = luaL_len(L,-1) + 1;
			set_nn = 1;
		}

	}
	//add or remove?
	if (set_nn){
		nml_set_or_error(L, self->socket, prop, &buf, str_sz, buf);
		// printf("%s to '%s'.\n",exists ? "Unsubscribed" : "Subscribed", buf);
		// dump_stack(L, "Successfully set it!");
	}

	lua_settop(L, 1);
	return ret_count;
}

int nml_subscribe__newindex (lua_State * L) 
{

	lua_getmetatable(L, 1);
	lua_getfield(L, -1, "__socket");
	lua_insert(L, 1); //socket ud is now in first position, as god intended.
	lua_pop(L,1); //pop metatable

	//strings have to fit the NML_SUB_STRING_MAX
	if(luaL_len(L, -2) > NML_SUB_STRING_MAX - 1)
		luaL_error(L, "'%s' exceeds the maximum length of a subscription string, which is %d",
			lua_tostring(L, -2), NML_SUB_STRING_MAX - 1);

	return do_sub_newindex(L);
}


int nml_set_subscribe (lua_State * L)
{
	int 	ret_count;
	int 	i;
	int 	len;

	ret_count = 0;

	lua_getfield(L, 1, "subscriptions");
	lua_insert(L, -2); //put __subscriptions behind the value that we want to set it to.
	//dump_stack(L, "nml_set_subscribe: 'subscriptions', subscriptions (table), new_value");

	if(lua_isnil(L, -1)){ //clear out all subscriptions.

		lua_pop(L,1); // pop nil.
		lua_pushvalue(L, -1); //copy the table. pairs will eat it.
		luaL_getmetafield(L,-1,"__pairs");
		// dump_stack(L, "pairs retrieved.");
		lua_insert(L, -2);//put it behind the table.
		lua_call(L,1,3);//callpairs
		// dump_stack(L, "after pairs is called");

		//make a copy of the pairs function
		lua_pushvalue(L, -3);
		lua_insert(L, -3);
		lua_pushvalue(L,-2);
		lua_insert(L,-4);
		// dump_stack(L, "returned. back in nml_set");
		

		// lua_call(L, 2,2);
		// dump_stack(L, "first call to pairs");
		// luaL_error(L, "Break");

		for(lua_call(L, 2,2); !lua_isnil(L,-1); lua_call(L, 2,2)){

			// dump_stack(L, "wiping table. pairs returned");
			lua_pop(L,1);
			lua_pushnil(L);
			//dump_stack(L, "ready to set table?");
			lua_settable(L, -3);
			// dump_stack(L, "after settable wipe loop");
			lua_pushvalue(L, -2);
			lua_pushvalue(L, -2);
			lua_insert(L, -2);
			lua_insert(L, -2);
			lua_pushnil(L);
			// dump_stack(L, "restarting wipe loop");
			// luaL_error(L, "Break");
			
		}
		lua_settop(L, lua_gettop(L) - 4); lua_pushnil(L);
		
		// dump_stack(L, "finished cleaning table.");

		
		
	}else if(lua_isstring(L, -1)) {

		lua_tostring(L, -1); //convert number  to string

		lua_newtable(L);
		lua_pushnumber(L, 1);
		lua_pushvalue(L, -3); //this should be the string.
		// dump_stack(L, "position -4 and -1 should be the name");
		lua_settable(L, -3);
		//call this function again, but now with the value in a table.
		nml_set_subscribe(L);
		//subscriptions and string value should now be on top.
	}else if(lua_istable(L,-1)) {

		//call this function and set subscription to nil.
		lua_pushnil(L);
		nml_set_subscribe(L);
		len = (int)luaL_len(L,-1);
		for(i = 1; i <= len; i++){
			lua_rawgeti(L, -1, i);
			//string value on top?
			if(!lua_isstring(L, -1))
				luaL_error(L, "each value must be a string or a number. Received: %s", lua_typename(L, lua_type(L,-1)));
			lua_pushboolean(L, 1); 
			//dump_stack(L, "stack: ..., subscription table, new table, key, value");
			lua_settable(L, -4);

		}
	}else 
		luaL_error(L, "expected a table, string or nil. Received %s", lua_typename(L, lua_type(L, -1)));

	lua_remove(L, -1); lua_remove(L, -1);//remove the subscriptions table and value, as is customary.
	//should be returning with the same stack in place, - the value that we're setting it to.
	return ret_count;
}
//__index of sub.subscriptions 
int nml_get_subscribe (lua_State * L)
{
	int 	ret_count; 

	ret_count = 1;

	// nml_socket_ud * self =  nml_get_socket(L);

	lua_getuservalue(L, 1);
	lua_pushstring(L, "__subscriptions");
	lua_rawget(L, -2);

	
	return ret_count;
}
///////////////////////////////// helper functions for socket.c


//helper function for sockets: finds if they set the domain
//removes the raw key, if set.
//assumes that self (socket ud) is in first postion (unused) 
//table is in second (optional)

//THIS IS NOT RETURNING ARGUMENT COUNT. It returns the domain.
static int set_domain (lua_State *L) 
{
	int domain = AF_SP;

	if (!lua_istable(L, 2)) return domain;
	lua_getfield(L, 2, "raw");
	if(lua_toboolean(L, -1))
		domain = AF_SP_RAW;
	lua_pop(L, 1); //pop value.
	return domain;

}
//call_for_each_in_field
//looks for field_name in option table. turns string into table or 
//expects table that is a sequence of strings arguments.
//for each string in the sequence, calls a field_name method from self.
//stores return value as a table where {<string_argument>, <retvalue/false>, <errormessage/nil>}
//@param self
//@param option table
//@param field name
//@return nil or table


static int call_for_each_in_field (lua_State *L, const char * field_name)
{	
	//printf("field name is: %s\n", field_name);
	//dump_stack(L, "Enter stack dump");
	nml_socket_ud * self;
	if (lua_istable(L, 2)) 
		lua_getfield(L, 2, field_name);
	//dump_stack(L, "looked at second argument and got field");
	if(!lua_istable(L,2) || !lua_toboolean(L, -1))
	{
		lua_settop(L, 2);
		return 2; //return what I dun got in.
	}
	
	self = nml_get_socket(L);
	
	//turn the returned value into a table, if there
	//is only one value.
	if (lua_isstring(L, -1)) {
		lua_newtable(L);
		lua_insert(L, -2);
		lua_rawseti(L, -2, 1);
	}

	lua_checkstack(L, lua_gettop(L) + 6); //make sure we have room.

	luaL_checktype(L, -1, LUA_TTABLE); //is this a table?
	
	lua_newtable(L); //the  table used to show return values from calls
	lua_pushnil(L);
	//call next with nil.
	//dump_stack(L, "retrieve method from stack");
	while(lua_next(L, -3)){
		//dump_stack(L, "top of loop");
		// stack is: self, opt_table, ret_table, index, arg
		lua_pushvalue(L, 1); //copy the user value.
		lua_pushvalue(L, -2); //copy the arg

		lua_getfield(L, 1, field_name);//retrieve the socket's  method
		lua_insert(L, -3); //put it behind the copy of the arg
		//dump_stack(L, "what is the field name? stack");
		lua_call(L, 2, 2);

		// stack is: self, opt_table, args, ret_table, index, arg, retval, errmsg
		lua_createtable(L, 3, 0); //call_ret_table
		lua_insert(L, -4); 
		// stack is: self, opt_table, ret_table, index, 
		// call_ret_table, arg, retval, errmsg

		lua_rawseti(L,-4, 3); //rettable, set newtable[3]=errormsg
		lua_rawseti(L,-3, 2); //rettable, set newtable[2]=retval
		lua_rawseti(L,-2, 1); //rettable, set newtable[1]=arg

		// stack is: self, opt_table, ret_table, index, call_ret_table

		nml_rawsetnexti(L, -3);

		//stack is at: self, option table,  return_table, index,
	}
	//dump_stack(L, "map, out of loop, stack: self, opt, args, ret");
	//rettable table is at top of the stack.

	lua_remove(L, -2); //remove the args table
//	dump_stack(L, "map, remove args stack: self, opt, ret");
	//set the return value to the field name for the options table
	lua_setfield(L,-2, field_name);
//	dump_stack(L, "map, make field_name = retval table stack: self, opt");

	return 2;
}

static void set_all_props(lua_State *L) 
{
	nml_socket_ud * self;

	//no table, no options.
	if (!lua_istable(L, 2)) return;
	
	self = nml_get_socket(L);
	//set all of the properties
	//this will call __newindex, which uses a property getter/setter
	lua_pushnil(L);
	while(lua_next(L, 2)){

		lua_pushvalue(L, -2); //copy key
		lua_insert(L,-2); // put it behind the val
		lua_settable(L, 1); //set the ud value (which uses its metamethod nml_newindex)
		// new key is on top
	}
}

static int shutdown_all(lua_State *L)
{
	int ret_count; 
	nml_socket_ud * self;

	ret_count = 1;

	self =  nml_get_socket(L);
	
	//printf("at top of shutdown all with %d socket", self->socket);
	
	lua_settop(L,1); //let there be only one argument: self
	//nml_set_end_points_stack:
	//	gets uservalue[end_points], removes uservalue and then adds a nil to the end 
	lua_getuservalue(L,1);
	lua_getfield(L, 2, "end_points");
	lua_remove(L,2); 
	lua_pushnil(L);
	///nml_set_end_points_stack(L); 
	
	//loop on the end_points table...
	while (lua_next(L, 2) != 0) {
		//stack is: self, endpoints, eid (index), eid_table (value)
		//dump_stack(L, "shutdownall: self, endpoints, eid (index), eid_table (value)");
		//don't care what the value is. the index is the eid.
		//remove the index of the endpoints object and remove the endpoints table
		lua_remove(L, -2); lua_remove(L , -2);
		//lua_pop(L, 1);
		//lua_remove(L, 2); //remove the endpoints table.
		
		//call shutdown with: self, eid
		ret_count = nml_shutdown(L);
	//	printf("shutdown was called and returned with %d arguments.\n", ret_count);
		if (ret_count > 1) //an error was received.
			break; 
		lua_getuservalue(L,1);
		lua_getfield(L, 2, "end_points");
		lua_remove(L,2); 
		lua_pushnil(L);
	//nml_set_end_points_stack(L);
	}
	return ret_count;
}
static int freemsg(lua_State * L) {

	nml_socket_ud * self; 

	self =  nml_get_socket(L);

	if (self->len > 0){
		if(nn_freemsg(self->buf) == 0) {

			self->len = 0;
			self->buf = NULL;


		}
		else
			nml_error(L,"Error when freeing message");
	}

	return 1;

}


static struct property nml_get_nn_opt(const char * index)
{
	int i;
	for(i = 0; properties[i].name != NULL; i++) {
		if (strcmp(properties[i].name, index) == 0){
			return properties[i];
		}
	}
	return properties[i];
}

static void nml_set_or_error(lua_State * L, int s, struct property prop, void * v, 
	size_t sz, const char * value)
{
	char errstr[128];

	if (nn_setsockopt( s, prop.nn_level, prop.nn_name, v, sz ) != 0) {
		_snprintf_s( errstr, 127, _TRUNCATE,
	 		"Could not set %s value for socket with value '%s'.", prop.name,
			value
		);
		nml_error(L, errstr);
	}
}

static void nml_get_or_error(lua_State * L, int s, struct property prop, void * v, 
	size_t * sz)
{
	char errstr[128];

	if (nn_getsockopt(s, prop.nn_level, prop.nn_name, v, sz) != 0
	){
		_snprintf_s( errstr, 127, _TRUNCATE,
	 		"Could not get %s value for socket", prop.name);
		nml_error(L, errstr);
	}	
}

int nml_freemsg(lua_State* L)
{
	// {[1]=nn_msg*}
	luaL_checktype(L, 1, LUA_TTABLE);

	lua_pushinteger(L, 1);
	lua_gettable(L, -2);

	// void*
	luaL_checktype(L, -1, LUA_TUSERDATA);
	
	nn_freemsg(lua_touserdata(L, -1));

	// replace it with false
	lua_pushinteger(L, 1);
	lua_pushboolean(L, 0);
	lua_settop(L, -3);

	// return self
	lua_settop(L, 1);
	return 1;
}