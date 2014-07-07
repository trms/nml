#include "nml.h"

/***
Clones the specified message userdata into a new message userdata. 
@function msg_frommessage
@param the source nml message user data
@param the destination nml message user data 
@return the source nml message user data
or
@return nil
@return error message
*/
int l_msg_frommessage(lua_State* L)
{
	size_t sizePayload;
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");
	void** ppck2 = (void**)luaL_checkudata(L, 2, "nml_msg");

	// if there's already a payload, free it first
	if (*ppck2!=NULL) {
		if (ck_free(*ppck2)!=0) {
			lua_pushnil(L);
			lua_pushstring(L, nn_strerror(nn_errno()));
			return 2;
		}
	}
	*ppck2 = ck_alloc(ck_get_size(*ppck));

	// copy the data
	ck_copy_data(*ppck2, ck_get_data(*ppck), ck_get_size(*ppck));

	// set ckid
	ck_set_header(*ppck2, *((DWORD*)ck_get_header(*ppck)));

	lua_settop(L, 1);
	return 1;
}