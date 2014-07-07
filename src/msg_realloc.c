#include "nml.h"

/***
Reallocs an existing nml message data payload.
@function msg_realloc
@param the nml message user data
@param size the realloc size
@return the nml message userdata
or
@return nil
@return error message
*/
int l_msg_realloc(lua_State* L)
{
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");

	*ppck = ck_realloc(*ppck, (int)luaL_checkinteger(L, 2));

	if (*ppck==NULL) {
		lua_pushnil(L);
		lua_pushstring(L, nn_strerror(nn_errno()));
		return 2;
	}
	lua_settop(L, 1);
	return 1;
}