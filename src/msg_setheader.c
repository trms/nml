/*
    Copyright (c) 2014 Tightrope Media Systems inc.  All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "nml.h"

/// @module nml

/***
message api
@section message
*/

/***
Sets the nml message's header four character code. 
If the nml message doesn't contain a data payload this will allocate a new payload big enough to hold the ckid and cksize.
If there's a problem allocating the payload it will return an error message.
Supplied userdata buffer must be NML-allocated.
If the specified header is larger than four characters it will be truncated, and a warning will be returned as a second return value.
@function msg_setheader
@param msg_ud the nml message userdata
@tparam string the nml message's header string
@return the nml message userdata, or nil
@return nil, or a string containing a warning message (string truncated), or a string containing an error message
*/
int l_msg_setheader(lua_State* L)
{
	int istrlen;
	const char* pch = lua_tolstring(L, 2, &istrlen);
	void** ppck = (void**)luaL_checkudata(L, 1, g_achBufferUdMtName);

	if (*ppck==NULL) {
		// allocate a new data payload
		*ppck = ck_alloc(4); // make it non-zero

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