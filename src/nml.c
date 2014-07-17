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
NML stands for nanomsg library, and is a lua binding for nanomsg.
@module nml
@author Tightrope Media Systems
@copyright 2014
@license this software is licensed under the MIT license
*/

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