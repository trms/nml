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
	
	int iBytesReceived, iFlags=0;
	char* pData;

	if (isSocket(L, P1)==0)
		return pushError(L, g_achInvalidSocketParameter);

	// flags
	if (lua_type(L, P2)==LUA_TSTRING) {
		if (_tcscmp(lua_tostring(L, P2), _T("NN_DONTWAIT"))==0)
			iFlags = NN_DONTWAIT;
	}
	iBytesReceived = nn_recv((int)lua_tonumber(L, P1), &pData, NN_MSG, iFlags);

	if (iBytesReceived==-1)
		return pushErrorNo(L);

	// put the string in lua space
	lua_pushlstring(L, pData, iBytesReceived-1); // don't send the null terminator

	// free the nn buffer
	if (nn_freemsg(pData)==-1)
		return pushErrorNo(L);
	
	// return the string
	return 1;
}