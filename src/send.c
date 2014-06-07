#include "nml.h"

// send a message
// http://nanomsg.org/v0.3/nn_send.3.html
//
//	NOTE: this always expects a STRING message
//
int l_send(lua_State* L)
{
	int iBytesSent, iFlags=0;
	size_t sizeBuffer;
	void* pData;

	if (isSocket(L, P1)==0)
		return pushError(L, g_achInvalidSocketParameter);

	if (lua_type(L, P2)!=LUA_TSTRING)
		return pushError(L, _T("expected a string message format"));

	if (lua_type(L, P3)!=LUA_TNUMBER)
		return pushError(L, _T("expected a number size parameter"));

	if ((lua_type(L, P4)!=LUA_TSTRING)&&(lua_type(L, P4)!=LUA_TNUMBER))
		return pushError(L, _T("expected a string or number flags parameter"));

	// message
	lua_tolstring(L, P2, &sizeBuffer);
	
	// alloc the message, add the terminator since lua won't
	pData = nn_allocmsg(++sizeBuffer, 0); // use default allocator
	if (pData==NULL)
		return pushErrorNo(L);

	// copy the message
	memcpy(pData, lua_tostring(L, P2), sizeBuffer-1);

	// flags
	if (lua_type(L, P4)==LUA_TSTRING) {
		if (_tcscmp(lua_tostring(L, P4), _T("NN_DONTWAIT"))==0)
			iFlags = NN_DONTWAIT;
	} else if (lua_type(L, P4)==LUA_TNUMBER)
		iFlags = (int)lua_tonumber(L, P4);

	// this will free the pData
	iBytesSent = nn_send((int)lua_tonumber(L, P1), &pData, NN_MSG, iFlags);

	if (iBytesSent==-1)
		return pushErrorNo(L);

	assert(iBytesSent==sizeBuffer);

	lua_pushnumber(L, iBytesSent);
	return 1;
}