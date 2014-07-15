#include "nml.h"
/***
Allocates a new nml buffer payload.
If the buffer contains a header but has no data, this will free the existing header chunks, allocate a new buffer, and store the header info.
@function msg_alloc
@param the desired size
@return the message userdata
or
@return nil
@return error message
*/
int l_msg_alloc(lua_State* L)
{
   void** ppv = (void**)lua_newuserdata(L, sizeof(void*));

   *ppv = ck_alloc(luaL_checkint(L, -2));

   // pop the int, keep the ud
   lua_remove(L, -2);

   if (*ppv==NULL) {
      lua_pushnil(L);
      lua_pushstring(L, nn_strerror(nn_errno()));
      return 2;
   }
   // populate the ud metatable - also called by recv
   populatemessagemt(L);

   return 1;
}