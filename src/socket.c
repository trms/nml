/**
 * @file
 * @author  Christian Bechette <christian.bechette@trms.com>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 */

#include "nml.h"

int l_socket(lua_State* L)
{
	int iSocket, aiParams[2], i;

	// int nn_socket (int domain, int protocol);
	if ((lua_type(L, P1)!= LUA_TNUMBER)&&(lua_type(L, P1)!= LUA_TSTRING))
		return pushError(L, _T("domain parameter should be a number or a string"));

	else if ((lua_type(L, P2)!= LUA_TNUMBER)&&(lua_type(L, P2)!= LUA_TSTRING))
		return pushError(L, _T("protocol parameter should be a number or a string"));
	
	// domain
	if (lua_type(L, P1)==LUA_TSTRING) {
		if (stringToDomain(lua_tostring(L, P1), &(aiParams[0]))==0)
			return pushError(L, _T("unable to parse domain string"));
	}
	else {
		aiParams[0] = (int)lua_tonumber(L, P1);

		// validate the number
		if ((aiParams[0]!=AF_SP)&&(aiParams[0]!=AF_SP_RAW))
			return pushError(L, _T("unrecognized domain identifier"));
	}
	// protocol
	if (lua_type(L, P2)==LUA_TSTRING) {
		if (stringToProtocol(lua_tostring(L, P2), &(aiParams[1]))==0)
			return pushError(L, _T("unable to parse protocol string"));
	} else {
		aiParams[1] = (int)lua_tonumber(L, P2);

		// validate the protocol
		for (i=0; (i<g_iNanoMsgProtocols) && (aiParams[1]!=g_NanomsgProtocols[i].value); i++) {}
		
		if (i>=g_iNanoMsgProtocols)
			return pushError(L, _T("unrecognized protocol identifier"));
	}
	// return the newly created socket
	iSocket = nn_socket(aiParams[0], aiParams[1]);
	if (iSocket==-1)
		return pushErrorNo(L); // this will push errno

	// all good, return the newly created socket
	lua_pushnumber(L, iSocket);
	assert(isSocket(L, -1)==1);
	return 1;
}