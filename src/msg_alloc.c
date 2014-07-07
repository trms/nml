#include "nml.h"
/***
Allocates a new nml buffer payload.
If the buffer contains a header but has no data, this will free the existing header chunks, allocate a new buffer, and store the header info.
@function msg_alloc
@param the nml message userdata
@param the desired size
@return the message userdata
or
@return nil
@return error message
*/
int l_msg_alloc(lua_State* L)
{
	// use nanomsg's allocators
	void** ppck = (void**)luaL_checkudata(L, 1, "nml_msg");
	DWORD dwFourcc = 0;

	if (*ppck!=NULL) {
		if (ck_get_size(*ppck)>0) {
			dwFourcc = *(DWORD*)ck_get_header(*ppck);
			nn_freemsg(*ppck);
		}
	}

	*ppck = ck_alloc(luaL_checkint(L, 2));

	if (*ppck==NULL) {
		lua_pushnil(L);
		lua_pushstring(L, nn_strerror(nn_errno()));
		return 2;
	}
	// retain the header if any
	if (dwFourcc!=0)
		ck_set_header(*ppck, dwFourcc);

	lua_settop(L, 1);
	return 1;
}