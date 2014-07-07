#include "nml.h"

/***
Puts the supplied lua string in the nml message's data payload, sets the size, and sets the header type to "STR ". 
Any existing payload will be freed first.
An error will be returned in case of a problem while freeing the existing payload.
An error will be returned in case of a problem while allocating space for the string.
@function msg_tostring
@param the nml message user data
@return the nml message user data
or
@return nil
@return error message
*/
int l_msg_fromstring(lua_State* L)
{
	size_t sizePayload;
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");

	const char* pch = luaL_checklstring(L, 2, &sizePayload);

	// if there's already a payload, free it first
	if (*ppck!=NULL) {
		if (ck_free(*ppck)!=0) {
			lua_pushnil(L);
			lua_pushstring(L, nn_strerror(nn_errno()));
			return 2;
		}
	}
	*ppck = ck_alloc(sizePayload);

	// copy the string
	ck_copy_data(*ppck, pch, sizePayload);

	// set ckid
	ck_set_header(*ppck, MAKEFOURCC('S', 'T', 'R', ' '));

	lua_settop(L, 1);
	return 1;
}