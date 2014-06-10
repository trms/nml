#include "nml.h"

// send a message
// http://nanomsg.org/v0.3/nn_send.3.html
//
//	NOTE: this always expects a STRING message
//
int l_send(lua_State* L)
{
	size_t sizeBuffer;
	char* pData;

	luaL_checktype(L, P1, LUA_TNUMBER); // the socket
	luaL_checktype(L, P2, LUA_TSTRING); // the message, a string
	luaL_checktype(L, P3, LUA_TNUMBER); // flags

	// message
	lua_tolstring(L, P2, &sizeBuffer);
	
	// alloc the message, add the terminator since lua won't
	pData = nn_allocmsg(++sizeBuffer, 0); // use default allocator
	if (pData!=NULL) {
		// copy the message
		memcpy(pData, lua_tostring(L, P2), sizeBuffer-1);

		// add the terminator
		pData[sizeBuffer-1] = '\0';

		// this will free the pData
		lua_pushnumber(L, nn_send((int)lua_tonumber(L, P1), &pData, NN_MSG, (int)lua_tonumber(L, P3)));
	} else
		lua_pushnil(L);
	return 1;	
}