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
Puts the supplied lua string in the nml message's data payload, sets the size, and sets the header type to "STR ".
Any existing payload will be freed first.
An error will be returned in case of a problem while freeing the existing payload.
An error will be returned in case of a problem while allocating space for the string.
@function msg_fromstring
@tparam string string the string to put into the message
@return the nml message userdata, or nil
@return nil, or a string containing an error message
*/
int l_msg_fromstring(lua_State* L)
{
	size_t sizePayload;
	const char* pch = luaL_checklstring(L, 1, &sizePayload);
   void** ppvDest;

   // get a new buffer
   l_nml_msg(L);

   // allocate the buffer
   lua_pushinteger(L, sizePayload);
   l_msg_alloc(L); // L:2

   // recover the buffer
   if (lua_isnil(L, -1)==FALSE) {
      ppvDest = (void**)luaL_checkudata(L, -1, g_achBufferUdMtName);
      
      // copy the string
      ck_copy_data(*ppvDest, pch, sizePayload);

      // set the header
      ck_set_header(*ppvDest, MAKEFOURCC('S', 'T', 'R', ' '));
   }
   // return the ud
	return 1;
}