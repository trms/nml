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

// http://nanomsg.org/v0.3/nn_allocmsg.3.html
/***
Allocates a new buffer payload using nanomsg's allocator and wraps it in a lua userdata.
@function msg_alloc
@param size the realloc size
@return the nml message userdata
or
@return nil
@return error message
*/
int l_allocmsg(lua_State* L)
{
   void** ppv = (void**)lua_newuserdata(L, sizeof(void*));
   *ppv = ck_alloc(luaL_checkint(L, 1));

   if (*ppv==NULL) {
      lua_pushnil(L);
      lua_pushstring(L, nn_strerror(nn_errno()));
      return 2;
   }
   // populate the ud metatable
   luaL_newmetatable(L, g_achBufferUdMtName);

   lua_pushstring(L, "__gc");
   lua_pushcfunction(L, l_msg_free);
   lua_settable(L, -3);

   lua_pushstring(L, "__len");
   lua_pushcfunction(L, l_msg_getsize);
   lua_settable(L, -3);

   lua_pushstring(L, "__tostring");
   lua_pushcfunction(L, l_msg_tostring);
   lua_settable(L, -3);

   lua_pushstring(L, "type");
   lua_pushcfunction(L, l_msg_getheader);
   lua_settable(L, -3);

   lua_setmetatable(L, -2);

   return 1;
}