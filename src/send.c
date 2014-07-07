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

	int socket;
	// message
	//TODO: IF USERDATA, then it's allocated by
	//NML (or there is a flag that says how it's allocated) and we don't need to copy.
	//note: the luaL version always converts into a string.
	const char * msg;
	int flags;
	int result;
	void* pvChunk;
	
	struct nn_iovec iov;
	struct nn_msghdr hdr;
	
	socket = luaL_checkint(L, P1); // the socket
	flags = (!lua_isnoneornil (L, P3) ) ?  luaL_checkint(L, P3) : 0; // flags
	msg = luaL_tolstring(L, P2, &sizeBuffer); //puts a new string at -1

	// alloc the message, add the terminator since lua won't
	pvChunk = nn_allocmsg(sizeBuffer, 0);

	iov.iov_base = &pvChunk;
	iov.iov_len = NN_MSG;

	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;

#ifdef CONTROL
	hdr.msg_controllen = NN_MSG;
	pvChunk = nn_allocmsg(sizeof("my_protocol"), 0);
	hdr.msg_control = &pvChunk;
#else
	hdr.msg_controllen = 0;
	hdr.msg_control = NULL;
#endif

	if (iov.iov_base!=NULL) {
		// copy the message
		memcpy(*(void**)iov.iov_base, msg, sizeBuffer);
#ifdef CONTROL
		// copy the header
		memcpy(pvChunk, "my_protocol", sizeof("my_protocol"));
#endif
		// this will free the pData
		result = nn_sendmsg (socket, &hdr, flags);
		if (result != -1 )
			lua_pushinteger(L, result);
		else
			lua_pushnil(L);
	} else
		lua_pushnil(L);
	return 1;	
}