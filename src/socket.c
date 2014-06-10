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
	luaL_checknumber(L, P1); // domain
	luaL_checknumber(L, P2); // protocol

	// return the newly created socket
	lua_pushnumber(L, nn_socket((int)lua_tonumber(L, P1), (int)lua_tonumber(L, P2)));
	
	return 1;
}