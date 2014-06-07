#include "nml.h"

// poll a set of SP sockets for readability and/or writability
// http://nanomsg.org/v0.3/nn_poll.3.html
int l_poll(lua_State* L)
{
	int iPollsLen, iEventsLen, i, j, iResult, iTimeout=0;
	struct nn_pollfd* polls;

	if (lua_type(L, P1)!=LUA_TTABLE)
		return pushError(L, _T("expected a table of pollfd structures"));
	/*
	{
		{fd=, events={}, revents={}},
		{fd=, events={}, revents={}},
	},
	*/
	lua_len(L, P1);
	iPollsLen = (int)lua_tonumber(L, P1);
	lua_pop(L, 1);

	if (iPollsLen==0)
		return pushError(L, _T("table of pollfd structures not found (expected?)"));

	polls = malloc(sizeof(struct nn_pollfd)*iPollsLen);

	for (i=0; i<iPollsLen; i++) {
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
			if (lua_type(L, -1)==LUA_TTABLE) {
				lua_len(L, -1);
				iEventsLen=(int)lua_tonumber(L, -1);
				lua_pop(L, 1);
				for (j=0; j<iEventsLen; j++) {
					lua_pushnumber(L, i+1);
					lua_gettable(L, -2);
					if (lua_type(L, -1)==LUA_TSTRING) {
						if (_tcscmp(lua_tostring(L, -1), _T("NN_POLLIN")))
							polls[i].events |= NN_POLLIN;
						else if (_tcscmp(lua_tostring(L, -1), _T("NN_POLLOUT")))
							polls[i].events |= NN_POLLOUT;
					}
					lua_pop(L, 1);
				}
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}

	// timeout
	if (lua_type(L, P2)==LUA_TNUMBER)
		iTimeout = (int)lua_tonumber(L, P2);

	// call nn
	iResult = nn_poll(polls, iPollsLen, iTimeout);

	if (iResult!=-1) {
		// reuse the return table
		assert(lua_gettop(L)==1);
		for (i=0; i<iPollsLen; i++) {
			// next element
			lua_pushnumber(L, i+1);
			lua_gettable(L, -2);

			if (lua_type(L, -1)==LUA_TTABLE) {
				// the events
				lua_pushstring(L, "revents");
				lua_newtable(L);
				j=0;
				if ((polls[i].revents&NN_POLLIN)==NN_POLLIN){
					lua_pushnumber(L, ++j);
					lua_pushstring(L, "NN_POLLIN");
					lua_settable(L, -3);
				}
				if ((polls[i].revents&NN_POLLOUT)==NN_POLLOUT){
					lua_pushnumber(L, ++j);
					lua_pushstring(L, "NN_POLLOUT");
					lua_settable(L, -3);
				}
				lua_settable(L, -3);
			}
			lua_pop(L, 1);
		}
	}
	// free the mem
	free(polls);

	assert(lua_gettop(L)==1);
	lua_pushnumber(L, iResult);

	// return the table + the number of signaled events
	return 2;
}