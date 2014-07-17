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
Frees a nml message.
This frees the memory pointed to by the userdata, and not the userdata itself, which is managed by lua's garbage collection.
If the memory pointed to by the userdata is NULL this will return FALSE.
@function msg_free
@param msg_ud the message userdata
@return true if it freed data, false if it didn't have any data to free, or nil in case of error
@return nil, or a string containing an error message
*/
int l_msg_free(lua_State* L)
{
	int iRet=1;
   void** ppv = (void**)luaL_checkudata(L, 1, g_achBufferUdMtName);

   if (*ppv!=NULL) {
      if (ck_free(*ppv)==0)
         lua_pushboolean(L, TRUE);
      else {
         lua_pushnil(L);
         lua_pushstring(L, nn_strerror(nn_errno()));
         iRet=2;
      }
   } else
      lua_pushboolean(L, FALSE);
   *ppv = NULL;
   return 1;
}