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

// http://msdn.microsoft.com/en-ca/library/windows/desktop/ms740141(v=vs.85).aspx

void BuildCFdSet(lua_State* L, int in_iOffset, fd_set* out_pfdset)
{
	int j;

	// build the C fd_set table
	luaL_checktype(L, in_iOffset, LUA_TTABLE);

	out_pfdset->fd_count = (u_int)luaL_len(L, in_iOffset);
	for (j=0; j<out_pfdset->fd_count; j++) {
		lua_pushnumber(L, j+1);
		lua_gettable(L, in_iOffset);
		out_pfdset->fd_array[j]=luaL_checkint(L, -1);
		lua_pop(L, 1);
	}
}

void BuildLuaFdSet(lua_State* L, fd_set* in_pfdset)
{
	// build the lua fd_set table
	int j;
	lua_newtable(L);
	for (j=0; j<in_pfdset->fd_count; j++) {
		lua_pushnumber(L, j+1);
		lua_pushinteger(L, in_pfdset->fd_array[j]);
		lua_settable(L, -3);
	}
}

int l_FD_CLR(lua_State* L)
{
	// removes s from the set
	fd_set sockets;
	BuildCFdSet(L, P2, &sockets);

	FD_CLR(luaL_checkint(L, P1), &sockets);

	BuildLuaFdSet(L, &sockets);
	return 1; // the new table
}

int l_FD_ISSET(lua_State* L)
{
	// Nonzero if s is a member of the set. Otherwise, zero.
	fd_set sockets;
	BuildCFdSet(L, P2, &sockets);

	lua_pushinteger(L, FD_ISSET(luaL_checkint(L, P1), &sockets));
	return 1;
}

int l_FD_SET(lua_State* L)
{
	// Adds descriptor s to set.
	fd_set sockets;
	BuildCFdSet(L, P2, &sockets);

	FD_SET(luaL_checkint(L, P1), &sockets);

	BuildLuaFdSet(L, &sockets);
	return 1; // the new table
}

int l_FD_ZERO(lua_State* L)
{
	// initializes the file descriptor set to contain no file descriptors
	fd_set sockets;
	FD_ZERO(&sockets);
	BuildLuaFdSet(L, &sockets);
	return 1;
}