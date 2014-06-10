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
	luaL_checktype(L, P2, LUA_TNUMBER); // #table (here for consistency with the C api)
	luaL_checktype(L, P3, LUA_TNUMBER); // timeout

	// {{}, {}, ...}
	lua_len(L, P1);
	
	// use the smallest of the two
	iLen = (int)(lua_tonumber(L, -1)<lua_tonumber(L, P2)?lua_tonumber(L, -1):lua_tonumber(L, P2));
	lua_pop(L, 1);

	// malloc the buffer
	polls = malloc(sizeof(struct nn_pollfd)*iLen);
	
	if (polls!=NULL) {
		for (i=0; i<iLen; i++) {
			// next element
			lua_pushnumber(L, i+1);
			lua_gettable(L, -2);

			if (lua_type(L, -1)==LUA_TTABLE) {
				// the socket
				lua_pushstring(L, "fd");
				lua_gettable(L, -2);
				if (lua_type(L, -1)==LUA_TNUMBER)
					polls[i].fd = (int)lua_tonumber(L, -1);
				lua_pop(L, 1);

				// the events
				lua_pushstring(L, "events");
				lua_gettable(L, -2);
				polls[i].events = (short)lua_tonumber(L, -1);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}

		// call nn
		lua_pushnumber(L, nn_poll(polls, iLen, (int)lua_tonumber(L, P3)));
		if (lua_tonumber(L, -1)!=-1) {
			// reuse the return table
			lua_pushvalue(L, 1);
			for (i=0; i<iLen; i++) {
				// next element
				lua_pushnumber(L, i+1);
				lua_gettable(L, -2);

				lua_pushstring(L, "revents");
				lua_pushnumber(L, polls[i].revents);
				lua_settable(L, -3);

				lua_settable(L, -2);
			}
			// result + table
			iRet = 2;
		} else
			iRet = 1;

		// free the mem
		free(polls);
	}
	return iRet;
}