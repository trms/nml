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

/// @module nml

/***
Creates a new nml message user data.
The userdata serves as a container that stores the actual payload address. 
The payload address is initialized to NULL.
The payload is allocated at the C level and manipulated through nml's msg_xx functions.
The userdata is assigned a metatable with the name 'NML_RIFF', used by the C code to identify the userdata allocator.
@function nml_msg
@return a nml userdata object
@see nml_message
*/
int l_nml_msg(lua_State* L)
{
	// create a new nml message buffer
	void** ppv = (void**)lua_newuserdata(L, sizeof(void*));

	// initialize the data to NULL
	*ppv = NULL;

   // populate the ud metatable - also called by recv
   populatemessagemt(L);

	// return the ud
	return 1;
}