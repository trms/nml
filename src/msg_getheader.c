#include "nml.h"

/***
Returns the nml message's header as a lua string.
@function msg_getheader
@param self the nml message user data
@return the nml message's header string.
or
@return nil
@return error message
*/
int l_msg_getheader(lua_State* L)
{
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");

	if (*ppck==NULL) {
		lua_pushnil(L);
		lua_pushstring(L, nn_strerror(nn_errno()));
		return 2;
	}
	lua_pushlstring(L, (char*)(*ppck), 4);
	return 1;
}