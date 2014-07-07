#include "nml.h"

/***
Puts the supplied lua string in the nml message's data payload, sets the size, 
@function msg_tostring
@param self the nml message user data
@return the nml message user data
*/
int l_msg_fromstring(lua_State* L)
{
	size_t sizePayload;
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");

	const char* pch = luaL_checklstring(L, 2, &sizePayload);

	// there shouldn't already be a payload
	assert(*ppck==NULL);

	*ppck = nn_allocmsg(sizePayload+8, 0);

	// copy the string
	memcpy(((char*)*ppck)+8, pch, sizePayload);

	// set ckid
	*((uint32_t*)*ppck) = MAKEFOURCC('S', 'T', 'R', ' ');

	// set cksize
	*((uint32_t*)*ppck+4) = sizePayload;

	lua_settop(L, 1);
	return 1;
}