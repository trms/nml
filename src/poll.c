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
struct nn_pollfd_b {
	int fd;
	short events;
	short revents;
	};
int safe_getint( lua_State * L, const char * field_name,  struct nn_pollfd * polls) 
{
	int val;
	//BAD BAD BAD
	//TODO: getfield can error on __index. We should fix this!
	lua_getfield(L, -1, field_name);
	//lua_pushstring(L, field_name);
	//lua_rawget(L, -2); //rawget this. we can't do errors here.

	if (lua_isinteger(L, -1)) {
		val = (int) lua_tointeger(L, -1);
		lua_pop(L, 1);
		return val;
	}
	else {
		free(polls);
		//lua_error causes a longjump. so we won't free twice.
		return luaL_error(L, "Expected and integer in field '%s'. Received %s.", field_name, luaL_typename(L, -1));
	}

}

//void setint(lua_State *L, const char *field_name, int val, nn_pollfd *polls ){
void safe_setint(lua_State *L, const char *field_name, int val){
	lua_pushinteger(L, val);
	lua_setfield(L, -2, field_name); //rawget this. we can't do errors here.
}

// poll a set of SP sockets for readability and/or writability
// http://nanomsg.org/v0.3/nn_poll.3.html
int l_poll(lua_State* L)
{
	int iLen, i, iRet, time_out, count;
	struct nn_pollfd * polls;

	/*
	struct nn_pollfd {
		int fd;
		short events;
		short revents;
		};
	*/
	luaL_checktype(L, 1, LUA_TTABLE); // events table
	time_out = luaL_checkint(L, P2); // timeout
	lua_settop(L, 1);

	// {{}, {}, ...}
	iLen = (int)luaL_len(L, 1);

	if( iLen == 0) {
		lua_pushinteger(L, iLen);
		lua_pushvalue(L, 1);
		iRet = 2;
	} else {
	
		// malloc the buffer
		polls = (struct nn_pollfd *) malloc(sizeof(struct nn_pollfd)*iLen);
	
		if (polls!=NULL) {
			for (i=0; i<iLen; i++) {
				// next element
				lua_rawgeti(L, 1, i + 1);

				if (lua_type(L, -1)==LUA_TTABLE) {
					// the socket
					polls[i].fd = safe_getint(L, "fd", polls);		
					// the events
					polls[i].events = (short) safe_getint(L, "events", polls);
				} else {
					free(polls);
					luaL_error(L, "Expected a table at the index %d", i +1);
				}
				// pop the socket table.
				lua_pop(L, 1);
			} 

			// call nn, push the number of activated sockets
			count =  nn_poll(polls, iLen, time_out);
			
			if (count != -1) {
				//table is still at top.
				for (i=0; i<iLen; i++) {
					// next element
					lua_rawgeti(L, 1, i + 1);
					safe_setint(L, "revents", polls[i].revents);
					lua_pop(L, 1);
				}
				// truncate the table
				// lua_pushnumber(L, i+1); //what?
				// lua_pushnil(L);
				// lua_settable(L, -3);
				lua_pushinteger(L, count);
				lua_insert(L, 1);
				// count, table
				iRet = 2;
			} else {
				lua_pushnil(L);
				iRet = 1; //nml errord. so, we'll let nml figure out the error number.
			}

			// free the mem
			free(polls);
		}
	}
	return iRet;
}