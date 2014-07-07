#include "nml.h"
/***
Allocates a new nml buffer payload.
@function msg_alloc
@param self, the message userdata
@param size, the desired size
@return the message userdata
or
@return nil
@return error message
*/
int l_msg_alloc(lua_State* L)
{
	// use nanomsg's allocators
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");

	// ckid + cksize + data
	int iSize = luaL_checkint(L, 2) + 8;
	
	*ppck = nn_allocmsg(iSize, 0);

	if (*ppck==NULL) {
		lua_pushnil(L);
		lua_pushstring(L, nn_strerror(nn_errno()));
		return 2;
	}
	lua_settop(L, 1);
	return 1;
}