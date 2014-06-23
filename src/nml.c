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



int dump_stack (lua_State *L, const char * msg) {

	int i;
	int top = lua_gettop(L); /*depth of the stack*/

	printf("\n%s:\n--------\n", msg ? msg : "Dumping stack: ");
	for(i= 1; i <= top; i++) {
		int t = lua_type(L, i);
		printf("%d:\t", i);
		switch(t){
		case LUA_TSTRING: { /* strings */
			printf("'%s'", lua_tostring(L,i));
			break;
								}
		case LUA_TBOOLEAN: { /*boolean values*/
			printf(lua_toboolean(L,i) ? "true" : "false");
			break;
								 }
		case LUA_TNUMBER: { /* numbers */
			printf("%g", lua_tonumber(L, i));
			break;
								}
		default: { /*anything else*/
			printf("%s", lua_typename(L,t));
			break;
					}
		}
		printf("\n"); /* put in a separator */
	}
	printf("--------\n"); /* end of listing separator */
	return 0;
}

// single DLL entry point, exposes a lua binding
extern __declspec(dllexport) int luaopen_nml_core(lua_State* L)
{
	int i;

	// the nml binding is a table with a number of functions inside it
	lua_newtable(L);
	for (i=0; i<g_inmlApis; i++) {
		lua_pushstring(L, g_apchApi[i].name);
		lua_pushcfunction(L, g_apchApi[i].fn);
		lua_settable(L, -3);
	}
	// return the table
	return 1;
}

/*BOOL WINAPI DllMain(
	_In_  HINSTANCE hinstDLL,
	_In_  DWORD fdwReason,
	_In_  LPVOID lpvReserved
	)
{
	if ((fdwReason==DLL_PROCESS_ATTACH) && (lpvReserved==NULL)) {
	} else if ((fdwReason==DLL_PROCESS_DETACH) && (lpvReserved==NULL)){
	}
	return TRUE;
}*/