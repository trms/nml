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

// fine-grained alternative to nn_send
// http://nanomsg.org/v0.3/nn_sendmsg.3.html
int l_sendmsg(lua_State* L)
{
	
	void *msg;
	struct nn_msghdr hdr;
	struct nn_iovec iov;
    void *msg_control;
    size_t msg_controllen;

	size_t sizeBuffer;
	int socket, flags, result;
	// message
	//TODO: IF USERDATA, then it's allocated by
	//NML (or there is a flag that says how it's allocated) and we don't need to copy.
	//note: the luaL version always converts into a string.
	const char * msg_str;
	
	socket = luaL_checkint(L, P1); // the socket
	flags = (!lua_isnoneornil (L, P3) ) ?  luaL_checkint(L, P3) : 0; // flags
	msg_str = luaL_tolstring(L, P2, &sizeBuffer); //puts a new string at -1
	lua_pop(L, 1);

	//make the buffer
	msg = nn_allocmsg(sizeBuffer, 0);
	
	
	
	if (!msg) {
		lua_pushnil(L);
		return 1;
	}
	
	memcpy(msg, msg_str, sizeBuffer);

	msg_control = nn_allocmsg(6, 0);
	memcpy(msg_control, (const void *) "string", 6);
	
	iov.iov_base = &msg;
	iov.iov_len = NN_MSG;
	memset (&hdr, 0, sizeof (hdr));
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;
	hdr.msg_control = &msg_control;
	hdr.msg_controllen = NN_MSG;
	

	result = nn_sendmsg (socket, &hdr, 0);
	//send frees the buffers automatically.
	if (result != -1 ) {
		lua_pushinteger(L, result);
	}
	else {
		lua_pushnil(L);
	}

	
	return 1;
}