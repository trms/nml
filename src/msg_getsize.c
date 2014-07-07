#include "nml.h"

/***
Returns the nml message's data payload size in bytes.
@function msg_getsize
@param self the nml message user data
@return the nml message data payload size as an integer
or
@return nil
@return error message
*/
int l_msg_getsize(lua_State* L)
{
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");

	if (*ppck==NULL) {
		lua_pushnil(L);
		lua_pushstring(L, nn_strerror(nn_errno()));
		return 2;
	}
	lua_pushinteger(L, *(((uint32_t*)*ppck)+4));
	return 1;
}