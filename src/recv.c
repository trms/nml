#include "nml.h"

// recv a message
// http://nanomsg.org/v0.3/nn_recv.3.html
//
//	NOTE: this always expects a STRING message
// NOTE: this only supports NN_MSG, the caller has to free the buffer through freemsg
//
int l_recv(lua_State* L)
{
	// receive data using a nn-allocated buffer
	// push the received data to a lua string (new buffer) -- we expect to always receive strings
	// free the nn-alloc'd buffer
	//
	// NOTE: this creates a copy of the data into lua space, which is what we want in this case since we're dealing with command strings

	int iIntermediateResult;
	char* pData;

	luaL_checktype(L, P1, LUA_TNUMBER); // socket
	luaL_checktype(L, P2, LUA_TNUMBER); // timeout
	
	lua_pushnumber(L, nn_recv((int)lua_tonumber(L, P1), &pData, NN_MSG, (int)lua_tonumber(L, P2)));

	if (lua_tonumber(L, -1)!=-1) {
		// put the string in lua space
		lua_pushlstring(L, pData, (size_t)lua_tonumber(L, -1)); // don't send the null terminator

		// free the nn buffer
		iIntermediateResult = nn_freemsg(pData);
		assert(iIntermediateResult!=-1); // I don't want to return this to the caller but I should have the ability to spot this problem in debug

		// result + string
		return 2;
	}
	// result
	return 1;
}