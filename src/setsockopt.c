#include "nml.h"

// set a socket option
// http://nanomsg.org/v0.3/nn_setsockopt.3.html
int l_setsockopt(lua_State* L)
{
	int iLevel, iOption, iValue, i;

	//////////////////////////////////////////////////////////////////////////
	// validate arguments
	
	if (isSocket(L, P1)==0)
		return pushError(L, g_achInvalidSocketParameter);

	// 2nd parameter is the level
	if ((lua_type(L, P2)!=LUA_TNUMBER)&&(lua_type(L, P2)!=LUA_TSTRING))
		return pushError(L, _T("expected a number or string level parameter"));

	// 3rd parameter is the option type
	if ((lua_type(L, P2)!=LUA_TNUMBER)&&(lua_type(L, P2)!=LUA_TSTRING))
		return pushError(L, _T("expected a number or string option type parameter"));

	// 4rth parameter is the option value
	if (lua_type(L, P4)!=LUA_TNUMBER)
		return pushError(L, _T("expected a number value parameter"));

	//////////////////////////////////////////////////////////////////////////
	// populate the values
	
	// level
	if (lua_type(L, P2)==LUA_TSTRING)
		for (i=0; (i<g_iNanomsgLevels)&&(_tcscmp(lua_tostring(L, P2), g_NanomsgLevels[i].name)); i++) {}
	else
		for (i=0; (i<g_iNanomsgLevels)&&((int)lua_tonumber(L, P2)!=g_NanomsgLevels[i].value); i++) {}

	if (i>=g_iNanomsgLevels)
		return pushError(L, _T("unable to find a valid level parameter"));

	iLevel = g_NanomsgLevels[i].value;

	// option
	if (lua_type(L, P3)==LUA_TSTRING)
		for (i=0; (i<g_iNanomsgOptionTypes)&&(_tcscmp(lua_tostring(L, P2), g_NanomsgOptionTypes[i].name)); i++) {}
	else
		for (i=0; (i<g_iNanomsgOptionTypes)&&((int)lua_tonumber(L, P2)!=g_NanomsgOptionTypes[i].value); i++) {}

	if (i>=g_iNanomsgOptionTypes)
		return pushError(L, _T("unable to find a valid option parameter"));

	iOption = g_NanomsgOptionTypes[i].value;

	// value
	iValue = (int)lua_tonumber(L, P4);

	if (nn_setsockopt((int)lua_tonumber(L, P1), iLevel, iOption, &iValue, sizeof(iValue))==-1)
		return pushErrorNo(L);
	
	// return 0
	lua_pushnumber(L, 0);
	return 1;
}