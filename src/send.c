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
Sends a nml message over a SP socket.
If the message's payload allocator is 'NML_RIFF', meaning it was allocated through nml's msg_xx api, this will perform a zero-copy send, and nanomsg will free the payload. 
NML will also reset the userdata payload to NULL.
If the message's payload wasn't allocated by nml, this will copy the payload into a nml buffer before sending it out. NML will then call the userdata's free function, signaling it consumed the payload.
See <http://nanomsg.org/v0.3/nn_send.3.html> for more details on the send function.
@function send
@tparam integer socket the SP socket identifier
@param msg_ud the message userdata
@tparam integer flags the send flags
@return the message userdata, or nil
@return nil, or a string containing an error message
*/
int l_send(lua_State* L)
{
	int socket;
	
	// message
	int flags;
	int result;
	
	struct nn_iovec iov;
	struct nn_msghdr hdr;

   void** ppck;

   // supplied buffer must be a userdata
   luaL_checktype(L, P2, LUA_TUSERDATA);

   ppck = (void**)luaL_testudata(L, P2, g_achBufferUdMtName);

   // check the allocator type
   if (ppck!=NULL) {
      // it's a NML_RIFF
      *ppck = ck_get_raw(*ppck);
      
      iov.iov_len = NN_MSG;
   } else {
      // get the buffer as a lightuserdata, go through lua since we don't know the internal data structure
      luaL_getmetafield(L, P2, "getbuffer");
      lua_pushvalue(L, P2);
      lua_call(L, 2, 1);

      *ppck = lua_touserdata(L, -1);
      lua_pop(L, 1);

      // get the size
      luaL_getmetafield(L, P2, "getsize");
      lua_pushvalue(L, P2);
      lua_call(L, 1, 1);
      
      iov.iov_len = (size_t)lua_tointeger(L, -1);
      lua_pop(L, 1);
   }
   iov.iov_base = ppck;

	socket = luaL_checkint(L, P1); // the socket
	flags = (!lua_isnoneornil (L, P3) ) ?  luaL_checkint(L, P3) : 0; // flags

	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;

	hdr.msg_controllen = 0;
	hdr.msg_control = NULL;

	if (iov.iov_base!=NULL) {
		// this will free the pData
		result = nn_sendmsg (socket, &hdr, flags);

      // remove our reference to the data, it will be freed by the send call
      if (ppck!=NULL)
         *ppck = NULL;
      else {
         // free the buffer, to make the send consistent across different allocator types
         luaL_getmetafield(L, P2, "free");
         lua_pushvalue(L, P2);
         lua_call(L, 1, 0);
      }
		if (result != -1 )
			lua_pushinteger(L, result);
		else
			lua_pushnil(L);
	} else
		lua_pushnil(L);

	return 1;	
}