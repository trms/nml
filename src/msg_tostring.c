#include "nml.h"

/***
Puts the nml message's data payload in a lua string.
Supplied userdata buffer must be NML-allocated.
@function msg_tostring
@param the nml message user data
@return the nml message data payload as a lua string
or
@return nil
@return error message
*/
int l_msg_tostring(lua_State* L)
{
	void** ppck = (void**)luaL_checkudata(L, 1, g_achBufferUdMtName);

	lua_pushlstring(L, ck_get_data(*ppck), ck_get_size(*ppck));
	return 1;
}