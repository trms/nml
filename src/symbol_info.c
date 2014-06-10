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