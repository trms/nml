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

// set a socket option
// http://nanomsg.org/v0.3/nn_setsockopt.3.html
int l_setsockopt(lua_State* L)
{
	// socket
	// level
	// option type
	// option value
	void * opt_value;
	size_t opt_size;
	int iValue;
	int socket =	luaL_checkint(L, P1);
	int level =		luaL_checkint(L, P2);
	int option =	luaL_checkint(L, P3); 

	if (  lua_isnumber(L, P4) ) {
		iValue = (int) lua_tointeger(L, P4);
		opt_size = sizeof(iValue);
		opt_value = &iValue;
	}
	else if (lua_isstring(L, P4) ) {
		opt_value = (void *) lua_tolstring(L, P4, &opt_size);
		++opt_size; //lua always puts a nil after the string, but reports a size that does not include it.
		
		if (opt_size > NML_MAX_STR) {
			printf("opt_size %d\n", opt_size);
			lua_pushnil(L);
			lua_pushfstring(L, "nml error: The option value is greater than the maximum size allowed, %d.", NML_MAX_STR);
			return 2;
		}
	} else {
		lua_pushnil(L);
		lua_pushfstring(L, "nml error: The option value type '%s' is invalid.",
			lua_typename(L, lua_type(L, P4)));
		return 2;
	}
	
	lua_pushboolean(L, nn_setsockopt( socket, level, option, opt_value, opt_size) != -1 );

	return 1;
}