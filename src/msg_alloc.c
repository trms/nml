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
Allocates a new nml buffer payload.
If the buffer contains a header but has no data, this will free the existing header chunks, allocate a new buffer, and store the header info.
@function msg_alloc
@param msg_ud the message userdata object
@tparam integer size the desired size in bytes
@return the message userdata, or nil
@return nil, or a string containing an error message
*/
int l_msg_alloc(lua_State* L)
{
   uint32_t ui32Header = 0;
   void** ppv = (void**)luaL_checkudata(L, -2, g_achBufferUdMtName);

   // free any current payload, keep the existing header data
   if (*ppv!=NULL) {
      ui32Header = *(uint32_t*)ck_get_header(*ppv);
      ck_free(*ppv);
   }
   *ppv = ck_alloc(luaL_checkint(L, -1));

   if (ui32Header!=0)
      ck_set_header(*ppv, ui32Header);

   // pop the int, keep the ud
   lua_remove(L, -1);

   if (*ppv==NULL) {
      lua_pushnil(L);
      lua_pushstring(L, nn_strerror(nn_errno()));
      return 2;
   }
   return 1;
}