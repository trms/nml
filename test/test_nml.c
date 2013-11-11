#include <lauxlib.h>
#include <lualib.h>
#include <lua.h>
#include <stdio.h>

int main(void) {
	lua_State *L = luaL_newstate(); /*opens Lua*/
	luaL_openlibs(L); /*opens the standard libraries, into the lua state (L)*/
	printf( "%d", luaL_dofile (L, "./test_luanaomsg.lua"));
}
