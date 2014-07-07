#include "nml.h"

/***
Frees a nml message.
@function msg_free
@param self, the message user data
@return true if success
or
@return nil
@return error message
*/
int l_msg_free(lua_State* L)
{
	// free the allocated buffer if any
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_message");

	if (ppck != NULL) {
		if (nn_freemsg(*ppck)!=0) {
			lua_pushnil(L);
			lua_pushstring(L, nn_strerror(nn_errno()));
			return 2;
		}
	}
	lua_pushboolean(L, TRUE);
	return 1;
}