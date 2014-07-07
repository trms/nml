#include "nml.h"

/***
Puts the nml message's data payload in a lua string.
@function msg_tostring
@param self the nml message user data
@return the nml message data payload as a lua string
or
@return nil
@return error message
*/
int l_msg_tostring(lua_State* L)
{
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");

	lua_pushlstring(L, (char*)*ppck+8, *(((uint32_t*)*ppck)+1));
	return 1;
}