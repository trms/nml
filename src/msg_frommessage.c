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
Clones the specified message userdata into a new message userdata.
Sets the new message size to match the copied data's size.
The source message userdata doens't need to be an nml message, as long as it supports the message api.
@function msg_frommessage
@param source_msg_ud the source message userdata
@return the new nml message userdata, or nil
@return nil, or a string containing an error message
*/
int l_msg_frommessage(lua_State* L)
{
   void* pvSource;
   void** ppvDest;
   int iSourceSize;
   DWORD dwHeader = 0;
   const char* pchHeader;
   int iHeaderSize;
   
   // NOTE: I don't know if the message is nml-allocated, so I use the message api here
   // get the size
   luaL_getmetafield(L, -1, "getsize");
   lua_pushvalue(L, 1);
   lua_call(L, 1, 1);

   if (lua_isnil(L, -1)) {
      // error
   } else
      iSourceSize = (int)lua_tointeger(L, -1);
   lua_pop(L, 1);

   // get the buffer
   luaL_getmetafield(L, -1, "getbuffer");
   lua_pushvalue(L, 1);
   lua_call(L, 1, 1);

   if (lua_isnil(L, -1)) {
      // error
   } else
      pvSource = lua_touserdata(L, -1);
   lua_pop(L, 1);

   // get a new message
   l_nml_msg(L);

   // alloc the buffer payload
   lua_pushinteger(L, iSourceSize);
   l_msg_alloc(L);

   // get the new buffer
   // since I know it's a nml buffer I'll use a shortcut here
   ppvDest = (void**)lua_touserdata(L, -1);
   
   if ((pvSource!=NULL)&&(*ppvDest!=NULL)&&(iSourceSize>0)) {
      ck_copy_data(*ppvDest, pvSource, iSourceSize);

      // set ckid
      luaL_getmetafield(L, 1, "getheader");
      lua_pushvalue(L, 1);
      lua_call(L, 1, 1);

      if (lua_isnil(L, -1)==FALSE) {
         pchHeader = lua_tolstring(L, -1, &iHeaderSize);
         memcpy(&dwHeader, pchHeader, min(4, iHeaderSize));
         ck_set_header(*ppvDest, dwHeader);
      }
      lua_pop(L, 1);
   }
   // return the dest ud
	return 1;
}