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

/***
Creates a new nml message user data.
The userdata doesn't contain any data, and is populated later following a nml_alloc call. 
@return a nml buffer
*/
int l_nml_msg(lua_State* L)
{
	int i;

	// create a new nml message buffer
	uint32_t* pui32 = (uint32_t*)lua_newuserdata(L, sizeof(void*));

	// initialize the data to NULL
	*pui32 = 0;

	// populate the ud metatable
	luaL_newmetatable(L, "nml_msg");

	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, l_msg_free);
	lua_settable(L, -3);

	lua_pushstring(L, "__len");
	lua_pushcfunction(L, l_msg_getsize);
	lua_settable(L, -3);

	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);

	lua_pushstring(L, "__tostring");
	lua_pushcfunction(L, l_msg_tostring);
	lua_settable(L, -3);

	lua_pushstring(L, "type");
	lua_pushcfunction(L, l_msg_getheader);
	lua_settable(L, -3);

	// make the msg functions accessible through the metatable
	// this way 3rd party C modules can access buffer allocators etc.
	for (i=0; i<g_inmlMsgApis; i++) {
		lua_pushstring(L, g_apchMsgApi[i].name);
		lua_pushcfunction(L, g_apchMsgApi[i].fn);
		lua_settable(L, -3);
	}
	lua_setmetatable(L, -2);

	// return the ud
	return 1;
}