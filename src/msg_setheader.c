#include "nml.h"

/***
Sets the nml message's header four character code. 
If the nml message doesn't contain a data payload this will allocate a new payload big enough to hold the ckid and cksize.
If there's a problem allocating the payload it will return an error message.
@function msg_setheader
@param the nml message user data
@param the nml message's header string.
@return the nml message user data
or
@return the nml message user data
@return a warning message (string truncated)
or
@return nil
@return error message
*/
int l_msg_setheader(lua_State* L)
{
	int istrlen;
	const char* pch = lua_tolstring(L, 2, &istrlen);
	void** ppck = (void**)luaL_checkudata(L, 1, g_achBufferUdMtName);

	if (*ppck==NULL) {
		// allocate a new data payload
		*ppck = ck_alloc(0);

		if (*ppck==NULL) {
			lua_pushnil(L);
			lua_pushstring(L, nn_strerror(nn_errno()));
			return 2;
		}
	}
	// write the header type
	ck_set_header(*ppck, 
		MAKEFOURCC(
			istrlen>0?pch[0]:' ',
			istrlen>1?pch[1]:' ',
			istrlen>2?pch[2]:' ',
			istrlen>3?pch[3]:' '));
		
	if (istrlen>4) {
		assert(FALSE); // I don't expect this to happen at this time
		lua_pushvalue(L, 1);
		lua_pushstring(L, "warning, header string truncated to the first four characters"); // fourcc code
		return 2;
	}
	lua_settop(L, 1);
	return 1;
}