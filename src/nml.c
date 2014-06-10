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