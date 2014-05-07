#ifndef NML_SOCKET_INCLUDE 
#define NML_SOCKET_INCLUDE 1
#include "nml.h"
#include <lua.hpp>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//scalability protocol metatable defines

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((unsigned __int32)(unsigned __int8)(ch0) | ((unsigned __int32)(unsigned __int8)(ch1) << 8) | \
	((unsigned __int32)(unsigned __int8)(ch2) << 16) | ((unsigned __int32)(unsigned __int8)(ch3) << 24 ))
#endif

namespace NNML
{
	static const int g_iMaxNumberOfSections = 16;

	const int g_iNumberOfBuffers = 2; // +2+2;

	const size_t g_iBufferSize = 1920*1080*2;

	const unsigned __int32 g_fourCCRIFF = MAKEFOURCC('R', 'I', 'F', 'F');
	const unsigned __int32 g_fourCCNML = MAKEFOURCC('n', 'm', 'l', ' ');
	const unsigned __int32 g_fourCCSection = MAKEFOURCC('s', 'e', 'c', 't');
	const unsigned __int32 g_fourCControl = MAKEFOURCC('c', 't', 'r', 'l');

	static void stack_trace(lua_State* L, char* out_pch)
	{
		lua_Debug entry;
		int depth = 0; 
		char ach[9];

		strcat(out_pch, "<<<<<< lua_stacktrace\n");

		while (lua_getstack(L, depth, &entry))
		{
			int status = lua_getinfo(L, "Sln", &entry);
			
			if (entry.name && (entry.currentline!=-1)) {
				strcat(out_pch, entry.short_src);
				strcat(out_pch, "(");
				_itoa_s(entry.currentline, ach, 9, 10);
				strcat(out_pch, ach);
				strcat(out_pch, "): ");
				strcat(out_pch, entry.name ? entry.name : "?");
				strcat(out_pch, "\n");
			}
			depth++;
		}
		strcat(out_pch, ">>>>>>");
	}
};

struct nml_socket_ud {
	int socket;
	char * buf;
	size_t len;
};

#define nml_opt_err(L) luaL_error(L,"cannot access option on closed socket.")
#define nml_set_end_points_stack(L) do {lua_getuservalue(L,1);lua_getfield(L, 2, "end_points");lua_remove(L,2); lua_pushnil(L);} while(0)
#define nml_get_socket(L) (nml_socket_ud *) luaL_checkudata(L, 1, NML_SOCKET);

int nml_get_linger(lua_State *);
int nml_set_linger(lua_State *);
int nml_get_int(lua_State *);
int nml_set_int(lua_State *);
int nml_get_timeout(lua_State *);
int nml_set_timeout(lua_State *);
int nml_set_ipv4only (lua_State *);
int nml_get_ipv4only (lua_State *);
int nml_set_reconnect_interval_max (lua_State * );
int nml_get_reconnect_interval_max (lua_State * );
int nml_set_read_only (lua_State *);
int nml_get_domain (lua_State *);
int nml_get_protocol (lua_State *);
int nml_set_subscribe (lua_State *);
int nml_get_subscribe (lua_State *);
int nml_getfd (lua_State *);


int nml_subscribe__newindex (lua_State *);
int nml_subscribe__index (lua_State *);
int nml_subscribe__len (lua_State *);
int nml_subscribe__pairs (lua_State *);

int nml_generic_socket_constructor (lua_State *, 
	int , 
	const char * lnm_proto_name
);

//the socket constructors
int nml_pair (lua_State *);
int nml_bus (lua_State *L);
int nml_push (lua_State *L);
int nml_pull (lua_State *L);
int nml_req (lua_State *L);
int nml_rep (lua_State *L);
int nml_pub (lua_State *L);
int nml_sub (lua_State *L);
int nml_surveyor (lua_State *L);
int nml_respondent (lua_State *L);
int nml_freemsg(lua_State* L);

//the metatable defines.
int nml_newindex(lua_State *);
int nml_index(lua_State * );
int nml_socket_gc(lua_State *);


//all methods that might be used on a socket.
int nml_connect(lua_State *);
int nml_bind(lua_State *);
int nml_send(lua_State *);
int nml_recv(lua_State *);
int nml_close(lua_State *);
int nml_shutdown(lua_State *);

#endif