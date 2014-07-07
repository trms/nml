#include "nml.h"

/***
Frees a nml message.
@function msg_free
@param the nml message user data
@return true if it freed data
of
@return false if it didn't have any data to free
or
@return nil
@return error message
*/
int l_msg_free(lua_State* L)
{
	// free the allocated buffer if any
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");

	if (*ppck != NULL) {
		if (ck_free(*ppck)!=0) {
			lua_pushnil(L);
			lua_pushstring(L, nn_strerror(nn_errno()));
			return 2;
		}
		// reset the ptr
		*ppck = NULL;

		lua_pushboolean(L, TRUE);
	} else
		lua_pushboolean(L, FALSE);
	return 1;
}