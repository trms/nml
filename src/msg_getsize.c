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
Returns the nml message's data payload size in bytes.
Supplied userdata buffer must be NML-allocated.
The data payload size in bytes may not reflect the true allocated size, which may contain extra control bytes and padding, or may have been over-allocated for optimization reasons.
@function msg_getsize
@param msg_ud the nml message userdata
@treturn integer the nml message data payload size as an integer, or nil
@return nil, or a string containing an error message
*/
int l_msg_getsize(lua_State* L)
{
	void** ppck = (void**)luaL_checkudata(L, 1, g_achBufferUdMtName);

	if (*ppck==NULL) {
		lua_pushnil(L);
		lua_pushstring(L, nn_strerror(nn_errno()));
		return 2;
	}
	lua_pushinteger(L, ck_get_size(*ppck));
	return 1;
}