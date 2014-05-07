/*LuaDumper*/
#include <lua.hpp>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdio.h>
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
/*
static const struct luaL_Reg dev_utils [] = {
	{"dump_stack", dump_stack},
	{NULL, NULL}
};

int luaopen_dev_utils(lua_State *L){
	luaL_newmetatable(L, "dev_utils");
	luaL_newlib(L, dev_utils);
	return 1;
}
*/