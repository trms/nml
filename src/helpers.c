#include "nml.h"

void populatemessagemt(lua_State* L)
{
   // populate the ud metatable
   luaL_newmetatable(L, g_achBufferUdMtName);

   lua_pushstring(L, "__gc");
   lua_pushcfunction(L, l_msg_free);
   lua_settable(L, -3);

   lua_pushstring(L, "__len");
   lua_pushcfunction(L, l_msg_getsize);
   lua_settable(L, -3);

   lua_pushstring(L, "__tostring");
   lua_pushcfunction(L, l_msg_tostring);
   lua_settable(L, -3);

   lua_setmetatable(L, -2);
}