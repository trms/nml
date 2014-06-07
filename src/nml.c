#include "nml.h"

g_iNanoMsgProtocols = 0;
void populateProtocols()
{
	struct nn_symbol_properties sym;
	int i=0, iCount=0, iLen=sizeof(sym);
	
	while(nn_symbol_info(i++, &sym, iLen)) {
		if (sym.ns==NN_NS_PROTOCOL)
			g_iNanoMsgProtocols++;
	}
	// allocate the table
	g_NanomsgProtocols = malloc(sizeof(SNameValue)*g_iNanoMsgProtocols);

	// populate it
	i=0;
	while(nn_symbol_info(i++, &sym, iLen)) {
		if (sym.ns==NN_NS_PROTOCOL) {
			g_NanomsgProtocols[iCount].name = (TCHAR*)malloc(_tcslen(sym.name)*sizeof(TCHAR));
			memcpy(g_NanomsgProtocols[iCount].name, sym.name, _tcslen(sym.name)*sizeof(TCHAR));
			iCount++;
		}
	}
	assert(g_iNanoMsgProtocols==iCount);
}

void freeSymbolTables()
{
	int i;
	// protocols
	for (i=0; i<g_iNanoMsgProtocols; i++) {
		free(g_NanomsgProtocols[i].name);
	}
	free(g_NanomsgProtocols);
}

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

BOOL WINAPI DllMain(
	_In_  HINSTANCE hinstDLL,
	_In_  DWORD fdwReason,
	_In_  LPVOID lpvReserved
	)
{
	if ((fdwReason==DLL_PROCESS_ATTACH) && (lpvReserved==NULL)) {
		// alloc the symbol tables
		populateProtocols();

	} else if ((fdwReason==DLL_PROCESS_DETACH) && (lpvReserved==NULL)){
		// free the memory
		freeSymbolTables();
	}
	return TRUE;
}

int pushError(lua_State* L, const TCHAR* in_pchMessage)
{
	lua_settop(L, 0);

	// push nil, error string on the stack
	lua_pushnil(L);
	lua_pushstring(L, in_pchMessage);
	return 2;
}

int pushErrorNo(lua_State* L)
{
	TCHAR ach[255];
	int iError;

	lua_settop(L, 0);
	iError = nn_errno();
	_tcserror_s(ach, 255, iError);
	return pushError(L, ach);
}

int stringToDomain(const TCHAR* in_pch, int* out_pi)
{
	// 0 is an invalid value
	static const int s_iInvalidSocket = 0;

	assert(out_pi!=NULL);
	*out_pi = s_iInvalidSocket;

	// translate from "AFP_SP" or "AFP_SP_RAW" to a number
	if (_tcscmp(in_pch, "AF_SP")==0)
		*out_pi = AF_SP;
	else if (_tcscmp(in_pch, "AF_SP_RAW")==0)
		*out_pi = AF_SP_RAW;
	
	assert(*out_pi!=s_iInvalidSocket);
	return (*out_pi!=s_iInvalidSocket)?1:0;
}

int stringToProtocol(const TCHAR* in_pch, int* out_pi)
{
	// 0 is an invalid value
	int i;
	static const int s_iInvalidProtocol = 0;

	assert(out_pi!=NULL);
	*out_pi = s_iInvalidProtocol;

	for (i=0; i<g_iNanoMsgProtocols; i++) {
		if (_tcscmp(in_pch, g_NanomsgProtocols[i].name)==0) {
			*out_pi = g_NanomsgProtocols[i].value;
			break;
		}
	}
	assert(*out_pi!=s_iInvalidProtocol);
	return (*out_pi!=s_iInvalidProtocol)?1:0;
}

int isSocket(lua_State* L, const int in_iStackOffset)
{
	return (lua_type(L, in_iStackOffset)==LUA_TNUMBER)?1:0;
}