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

// query the names and properties of nanomsg symbols
// http://nanomsg.org/v0.3/nn_symbol_info.3.html
int l_symbolinfo(lua_State* L)
{
	struct nn_symbol_properties sym;
	int opt_n = luaL_checkint(L, P1);
	int bytes;
	lua_pop(L, 1);
	bytes = nn_symbol_info(opt_n, &sym, sizeof(sym));

	if (bytes !=0) {
		lua_newtable(L);
		
		lua_pushstring(L, "value");
		lua_pushnumber(L, sym.value);
		lua_settable(L, -3);

		lua_pushstring(L, "name");
		lua_pushstring(L, sym.name);
		lua_settable(L, -3);

		lua_pushstring(L, "ns");
		lua_pushnumber(L, sym.ns);
		lua_settable(L, -3);

		lua_pushstring(L, "type");
		lua_pushnumber(L, sym.type);
		lua_settable(L, -3);

		lua_pushstring(L, "unit");
		lua_pushnumber(L, sym.unit);
		lua_settable(L, -3);

	}
	else {
		lua_pushnil(L);
	}
	return 1;
}