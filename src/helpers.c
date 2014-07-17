#include "nml.h"

/***
NML message userdata api.
@module nml_message
@author Tightrope Media Systems
@copyright 2014
@license this software is licensed under the MIT license
*/

void populatemessagemt(lua_State* L)
{
   // populate the ud metatable
   luaL_newmetatable(L, g_achBufferUdMtName);

   /***
   Garbage collection callback, implemented internally by calling nml.msg_free.
   @function __gc
   @see nml.msg_free
   */
   lua_pushstring(L, "__gc");
   lua_pushcfunction(L, l_msg_free);
   lua_settable(L, -3);

   /***
   Returns the nml message's data payload size in bytes.
   @function __len
   @see nml.msg_getsize
   */
   lua_pushstring(L, "__len");
   lua_pushcfunction(L, l_msg_getsize);
   lua_settable(L, -3);

   /***
   Puts the nml message's data payload in a lua string.
   @function tostring
   @see nml.msg_tostring
   */
   lua_pushstring(L, "__tostring");
   lua_pushcfunction(L, l_msg_tostring);
   lua_settable(L, -3);

   /***
   Lua ___index metatable implementation, points to the metatable.
   @function __index
   */
   lua_pushstring(L, "__index");
   lua_pushvalue(L, -2);
   lua_settable(L, -3);

   /***
   Lua __newindex implementation, points to the metatable.
   @function __newindex
   */
   lua_pushstring(L, "__newindex");
   lua_pushvalue(L, -2);
   lua_settable(L, -3);

   /***
   Allocates a new nml buffer payload.
   @function alloc
   @see nml.msg_alloc
   */
   lua_pushstring(L, "alloc");
   lua_pushcfunction(L, l_msg_alloc);
   lua_settable(L, -3);
   
   /***
   Reallocs an existing nml message data payload.
   @function realloc
   @see nml.msg_realloc
   */
   lua_pushstring(L, "realloc");
   lua_pushcfunction(L, l_msg_realloc);
   lua_settable(L, -3);

   /***
   Frees a nml message.
   @function free
   @see nml.msg_free
   */
   lua_pushstring(L, "free");
   lua_pushcfunction(L, l_msg_free);
   lua_settable(L, -3);

   /***
   Returns the nml message's header as a lua string.
   @function type
   @see nml.msg_getheader
   */
   lua_pushstring(L, "type");
   lua_pushcfunction(L, l_msg_getheader);
   lua_settable(L, -3);

   /***
   Sets the nml message's data payload size in bytes.
   @function setsize
   @see nml.msg_setsize
   */
   lua_pushstring(L, "setsize");
   lua_pushcfunction(L, l_msg_setsize);
   lua_settable(L, -3);

   lua_setmetatable(L, -2);
}

int dump_stack (lua_State *L, const char * msg)
{
   int i;
   int top = lua_gettop(L); /*depth of the stack*/

   printf("\n%s:\n--------\n", msg ? msg : "Dumping stack: ");
   for(i= 1; i <= top; i++) {
      int t = lua_type(L, i);
      printf("%d:\t", i);
      switch(t){
      case LUA_TSTRING:  /* strings */
         printf("'%s'", lua_tostring(L,i));
         break;
      case LUA_TBOOLEAN:  /*boolean values*/
         printf(lua_toboolean(L,i) ? "true" : "false");
         break;
      case LUA_TNUMBER:  /* numbers */
         printf("%g", lua_tonumber(L, i));
         break;
      case LUA_TUSERDATA:
         printf("%s - 0x%08X", lua_typename(L,t), lua_touserdata(L, i));
         break;
      case LUA_TLIGHTUSERDATA:
         printf("lightuserdata - 0x%08X", lua_touserdata(L, i));
         break;
      default:  /*anything else*/
         printf("%s", lua_typename(L,t));
         break;
      }
      printf("\n"); /* put in a separator */
   }
   printf("--------\n"); /* end of listing separator */
   return 0;
}