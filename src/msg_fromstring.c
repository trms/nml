#include "nml.h"

/***
Puts the supplied lua string in the nml message's data payload, sets the size, and sets the header type to "STR ". 
Any existing payload will be freed first.
An error will be returned in case of a problem while freeing the existing payload.
An error will be returned in case of a problem while allocating space for the string.
@function msg_fromstring
@param the nml message user data
@param the string to put into the message
@return the new nml message user data
or
@return nil
@return error message
*/
int l_msg_fromstring(lua_State* L)
{
	size_t sizePayload;
	const char* pch = luaL_checklstring(L, 1, &sizePayload);
   void** ppvDest;

   // allocate the buffer
   lua_pushinteger(L, sizePayload);
   l_msg_alloc(L); // L:2

   // recover the buffer
   if (lua_isnil(L, -1)==FALSE) {
      ppvDest = (void**)luaL_checkudata(L, -1, g_achBufferUdMtName);
      
      // copy the string
      ck_copy_data(*ppvDest, pch, sizePayload);

      // set the header
      ck_set_header(*ppvDest, MAKEFOURCC('S', 'T', 'R', ' '));
   }
   // return the ud
	return 1;
}