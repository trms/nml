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

// send a message
// http://nanomsg.org/v0.3/nn_send.3.html
//
//	NOTE: this always expects a STRING message
//
int l_send(lua_State* L)
{
	size_t sizeBuffer;
	void* pData;

	int socket = luaL_checkint(L, P1); // the socket
	// message
	//TODO: IF USERDATA, then it's allocated by
	//NML (or there is a flag that says how it's allocated) and we don't need to copy.
	//note: the luaL version always converts into a string.
	const char * msg = luaL_tolstring(L, P2, &sizeBuffer);

	int flags = luaL_optint(L, P3, 0); // flags
	int result;

	
	
	
	// alloc the message, add the terminator since lua won't
	pData = nn_allocmsg(sizeBuffer, 0); // use default allocator
	if (pData!=NULL) {
		// copy the message
		memcpy(pData, msg, sizeBuffer);
		
		// this will free the pData
		result = nn_send(socket, &pData, NN_MSG, flags);
		if (result != -1 )
			lua_pushinteger(L, result);
		else
			lua_pushnil(L);
	} else
		lua_pushnil(L);
	return 1;	
}