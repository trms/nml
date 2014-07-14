#include "nml.h"

/***
Frees a nml message.
@function msg_free
@param the nml message user data
@return true if it freed data
of
@return false if it didn't have any data to free
or
@return nil
@return error message
*/
int l_msg_free(lua_State* L)
{
	int iRet=1;
   void** ppv = (void**)luaL_checkudata(L, 1, g_achBufferUdMtName);

   if (*ppv!=NULL) {
      if (ck_free(*ppv)==0)
         lua_pushboolean(L, TRUE);
      else {
         lua_pushnil(L);
         lua_pushstring(L, nn_strerror(nn_errno()));
         iRet=2;
      }
   } else
      lua_pushboolean(L, FALSE);
   *ppv = NULL;
   return 1;
}