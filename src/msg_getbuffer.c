#include "nml.h"

/***
Returns the nml message's data payload address.
@function msg_getbuffer
@param the nml message userdata
@return the nml message's data payload address.
or
@return nil
@return error message
*/
int l_msg_getbuffer(lua_State* L)
{
	void** ppck = luaL_checkudata(L, 1, g_achBufferUdMtName);

	// return the payload address as a lud
	lua_pushlightuserdata(L, ck_get_data(*ppck));
	return 1;
}