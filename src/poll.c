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

// poll a set of SP sockets for readability and/or writability
// http://nanomsg.org/v0.3/nn_poll.3.html
int l_poll(lua_State* L)
{
	int iLen, i, iRet;
	struct nn_pollfd* polls;

	/*
	struct nn_pollfd {
		int fd;
		short events;
		short revents;
		};
	*/
	luaL_checktype(L, P1, LUA_TTABLE); // events table
	luaL_checkint(L, P2); // timeout

	// {{}, {}, ...}
	iLen = (int)luaL_len(L, P1);
	
	// malloc the buffer
	polls = malloc(sizeof(struct nn_pollfd)*iLen);
	
	if (polls!=NULL) {
		for (i=0; i<iLen; i++) {
			// next element
			lua_pushinteger(L, i+1);
			lua_gettable(L, 1);

			if (lua_type(L, -1)==LUA_TTABLE) {
				// the socket
				lua_pushstring(L, "fd");
				lua_gettable(L, -2);
				polls[i].fd = luaL_checkint(L, -1);
				lua_pop(L, 1);
				
				// the events
				lua_pushstring(L, "events");
				lua_gettable(L, -2);
				polls[i].events = (short)luaL_checkint(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}

		// call nn
		lua_pushinteger(L, nn_poll(polls, iLen, (int)lua_tointeger(L, P3)));
		if (lua_tointeger(L, -1)!=-1) {
			// reuse the return table
			lua_pushvalue(L, 1);
			for (i=0; i<iLen; i++) {
				// next element
				lua_pushinteger(L, i+1);
				lua_gettable(L, -2);

				lua_pushstring(L, "revents");
				lua_pushinteger(L, polls[i].revents);
				lua_settable(L, -3);

				lua_pop(L, 1);
			}
			// truncate the table
			lua_pushnumber(L, i+1);
			lua_pushnil(L);
			lua_settable(L, -3);

			// result + table
			iRet = 2;
		} else
			iRet = 1;

		// free the mem
		free(polls);
	}
	return iRet;
}