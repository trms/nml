#include "nml.h"

/***
Clones the specified message userdata into a new message userdata.
Sets the new message size to match the copied data's size.
@function msg_frommessage
@param the source nml message user data
@param the destination nml message user data 
@return the source nml message user data
or
@return nil
@return error message
*/
int l_msg_frommessage(lua_State* L)
{
   void* pvSource;
   void** ppvDest;
   int iSourceSize;
   DWORD dwHeader = 0;
   const char* pchHeader;
   int iHeaderSize;
   
   // NOTE: I don't know if the message is nml-allocated, so I use the message api here
   // get the size
   luaL_getmetafield(L, -1, "getsize");
   lua_pushvalue(L, 1);
   lua_call(L, 1, 1);

   if (lua_isnil(L, -1)) {
      // error
   } else
      iSourceSize = (int)lua_tointeger(L, -1);
   lua_pop(L, 1);

   // get the buffer
   luaL_getmetafield(L, -1, "getbuffer");
   lua_pushvalue(L, 1);
   lua_call(L, 1, 1);

   if (lua_isnil(L, -1)) {
      // error
   } else
      pvSource = lua_touserdata(L, -1);
   lua_pop(L, 1);

   // get a new message
   l_nml_msg(L);

   // alloc the buffer payload
   lua_pushinteger(L, iSourceSize);
   l_msg_alloc(L);

   // get the new buffer
   // since I know it's a nml buffer I'll use a shortcut here
   ppvDest = (void**)lua_touserdata(L, -1);
   
   if ((pvSource!=NULL)&&(*ppvDest!=NULL)&&(iSourceSize>0)) {
      ck_copy_data(*ppvDest, pvSource, iSourceSize);

      // set ckid
      luaL_getmetafield(L, 1, "getheader");
      lua_pushvalue(L, 1);
      lua_call(L, 1, 1);

      if (lua_isnil(L, -1)==FALSE) {
         pchHeader = lua_tolstring(L, -1, &iHeaderSize);
         memcpy(&dwHeader, pchHeader, min(4, iHeaderSize));
         ck_set_header(*ppvDest, dwHeader);
      }
      lua_pop(L, 1);
   }
   // return the dest ud
	return 1;
}