#include "nml.h"

/***
Returns the nml message's header as a lua string. If the nml message payload is not initialized this will return an empty string.
@function msg_getheader
@param the nml message user data
@return the nml message's header string.
*/
int l_msg_getheader(lua_State* L)
{
	void** ppck = (void**)luaL_checkudata(L, 1, g_achBufferUdMtName);

	if (  (*ppck==NULL)
      || (memcmp(ck_get_header(*ppck), "", 4)==0))
		lua_pushstring(L, "");
	else
		lua_pushlstring(L, ck_get_header(*ppck), 4);
	return 1;
}