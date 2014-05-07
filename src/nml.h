#ifndef NML_INCLUDE
#define NML_INCLUDE 1
#include "socket.h"
#include <string.h>
#include <nn.h>
#include <pair.h>
#include <pubsub.h>
#include <reqrep.h>
#include <pipeline.h>
#include <survey.h>
#include <bus.h>
#include <lua.hpp>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
//VISUAL LEAK DETECTOR
//only loads in debug mode. no need to guard it, but it will crash outside of VS, so....
#ifdef VLD
#include <vld.h>
#endif
int dump_stack (lua_State *, const char *);
//#include "dev_utils.c"

#ifdef _MSC_VER

#define snprintf c99_snprintf

#endif

#define NML_FD_TYPE FILE *
#define NML_SUB_STRING_MAX 64


#define NML_LIB "nml"
#define NML_SOCKET NML_LIB ".socket"

#define nml_rawsetnexti(L, index) lua_rawseti(L, index, lua_rawlen(L, index) + 1)
#define nml_error(L, msg) luaL_error(L, "%s:\n\t%s", msg, nn_strerror(nn_errno()))
#define nml_errorstr() nn_strerror(nn_errno())

extern const struct luaL_Reg nml_socket_metamethods [];

extern const int  NML_NN_DOMAINS[];
extern const char NML_DOMAIN_STRINGS[][16];

extern const int  NML_NN_PROTOCOLS [];
extern const char NML_PROTOCOL_STRINGS[][16];
extern const int NML_NN_OPTION_DEFAULTS [];
extern const int NML_OPTION_LEVELS [];
extern const char NML_OPTION_STRINGS[][32];
extern const int NML_OPTIONS [];


#ifdef _WIN32
#define DLLEXPORT  __declspec(dllexport)
#else
#define DLLEXPORT //empty
#endif
#endif