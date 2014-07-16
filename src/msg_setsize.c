#include "nml.h"

/***
Sets the nml message's data payload size in bytes.
Supplied userdata buffer must be NML-allocated.
The allocated buffer may only be partially filled. In this calling setsize with the valid data size after having called alloc will make the __len function report the valid size.
Setting a valid size smaller than the actual buffer size does not affect the free function.
Alloc will reset the buffer size to the size passed to the alloc function.
The buffer size is stored as part of the internal RIFF data structure.
@function msg_setsize
@param the nml message user data
@param the size integer in bytes
@return the nml message userdata
or
@return nil
@return error message
*/
int l_msg_setsize(lua_State* L)
{
   void** ppck = (void**)luaL_checkudata(L, 1, g_achBufferUdMtName);

   if (*ppck==NULL) {
      lua_pushnil(L);
      lua_pushstring(L, nn_strerror(nn_errno()));
      return 2;
   }
   ck_set_size(*ppck, (uint32_t)luaL_checkinteger(L, 2));
   lua_settop(L, 1);
   return 1;
}