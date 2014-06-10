#include "nml.h"

// query the names and properties of nanomsg symbols
// http://nanomsg.org/v0.3/nn_symbol_info.3.html
int l_symbolinfo(lua_State* L)
{
	struct nn_symbol_properties sym;

	luaL_checktype(L, P1, LUA_TNUMBER); // index

	lua_pushnumber(L, nn_symbol_info((int)lua_tonumber(L, P1), &sym, sizeof(sym)));
	if (lua_tonumber(L, -1)!=0) {
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

		// bytes + table
		return 2;
	}
	return 1;
}