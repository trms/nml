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

int l_select(lua_State* L)
{
	unsigned int i,j;
	struct timeval tv = {0};

	// read, write, except
	fd_set sockets[3] = {0};

	if (!lua_isnil (L, P4)) {
		luaL_checktype(L, P4, LUA_TTABLE);

		// set the tv
		lua_pushstring(L, "tv_sec");
		lua_gettable(L, P4);
		tv.tv_sec = (long)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "tv_usec");
		lua_gettable(L, P4);
		tv.tv_usec = (long)lua_tointeger(L, -1);
		lua_pop(L, 1);
	}
	// build the tables
	for (i=0; i<3; i++) {
		sockets[i].fd_count = (u_int)luaL_len(L, P1+i);
		for (j=0; j<sockets[i].fd_count; j++) {
			lua_pushnumber(L, j+1);
			lua_gettable(L, P1+i);
			sockets[i].fd_array[j]=luaL_checkint(L, -1);
			lua_pop(L, 1);
		}
	}
	// call the select
	lua_pushnumber(L, select(0, &(sockets[0]), sockets[1].fd_count==0?NULL:&(sockets[1]), sockets[2].fd_count==0?NULL:&(sockets[2]), lua_isnil(L, P4)?NULL:&tv));
	if (lua_tonumber(L, -1)!=SOCKET_ERROR) {
		// we'll return the tables
		// populate the tables
		for (i=0; i<3; i++) {
			lua_pushvalue(L, P1+i);
			sockets[i].fd_count = (u_int)luaL_len(L, -1);
			for (j=0; j<sockets[i].fd_count; j++) {
				lua_pushnumber(L, j+1);
				lua_pushinteger(L, sockets[i].fd_array[j]);
				lua_settable(L, -3);
			}
			// truncate the table, since we're reusing it
			lua_pushnumber(L, j+i);
			lua_pushnil(L);
			lua_settable(L, -3);
		}
		return 4;
	}
	return 1;
}