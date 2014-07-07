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

// recv a message
// http://nanomsg.org/v0.3/nn_recv.3.html
//
//	NOTE: this always expects a STRING message
// NOTE: this only supports NN_MSG, the caller has to free the buffer through freemsg
//
int l_recv(lua_State* L)
{
	// receive data using a nn-allocated buffer
	// push the received data to a lua string (new buffer) -- we expect to always receive strings
	// free the nn-alloc'd buffer
	//
	// NOTE: this creates a copy of the data into lua space, which is what we want in this case since we're dealing with command strings

	struct nn_iovec iov;
	struct nn_msghdr hdr;
	
	int iIntermediateResult;
	void* pData;
	void* pHeader;
	size_t sizeHeader;
	size_t sizeRecv;
	int flags = luaL_optint(L, P2, 0); // flags

	iov.iov_base = &pData;
	iov.iov_len = NN_MSG;

	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;
	hdr.msg_control = NULL; // &pHeader;
	hdr.msg_controllen = 0; // NN_MSG;

	sizeRecv = nn_recvmsg (luaL_checkint(L, P1), &hdr, flags);

	// socket
	// timeout
	//size_t buff_size =  nn_recv(luaL_checkint(L, P1), &pData, NN_MSG, flags);

	if (sizeRecv !=-1) {
		// put the string in lua space
		lua_pushlstring(L, (const char *) pData, sizeRecv);

		// free the nn buffer
		iIntermediateResult = nn_freemsg(pData);
		assert(iIntermediateResult!=-1); // I don't want to return this to the caller but I should have the ability to spot this problem in debug

		// put the header
		sizeHeader = nn_chunk_size(pHeader);
		lua_pushlstring(L, (const char*)pHeader, sizeHeader);

		iIntermediateResult = nn_freemsg(pHeader);
		assert(iIntermediateResult!=-1); // I don't want to return this to the caller but I should have the ability to spot this problem in debug

		return 2;
	} else
		lua_pushnil(L);
	// result
	return 1;
}