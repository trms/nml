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

#define NML_MAX_STR 256

// get a socket option
// http://nanomsg.org/v0.3/nn_getsockopt.3.html
int l_getsockopt(lua_State* L)
{

	size_t szValueSize;
	void * iValue;
	char iStr[NML_MAX_STR];
	int socket = luaL_checkint(L, P1);
	int level = luaL_checkint(L, P2);
	int option = luaL_checkint(L, P3);
	int option_type = luaL_checkint(L,4);
	
	if (option_type == NN_TYPE_STR){
		szValueSize = NML_MAX_STR;
		
		if(nn_getsockopt(socket, level, option, &iStr , &szValueSize) !=-1)
			lua_pushlstring(L,(const char *) iStr, szValueSize - 1);
		else
			lua_pushnil(L);
		
	}
	else {
		szValueSize = sizeof(int);
		if(nn_getsockopt(socket, level, option, &iValue , &szValueSize) !=-1)
			lua_pushinteger(L, *(int *) iValue);
		else
			lua_pushnil(L);
	}
	return 1;
}