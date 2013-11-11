#include <string.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/survey.h>
#include <nanomsg/bus.h>
#include <lauxlib.h>
#include <lualib.h>
#include <lua.h>
#include <stdio.h>
//#include "dev_utils.c"

#define NML_FD_TYPE FILE *

static int nml_socket (lua_State *);

#define NML_LIBNAME "nanomsg"
#define NML_SOCKET_TYPE NML_LIBNAME ".socket"
#define NML_DEFAULT_LINGER 1000
#define NML_DEFAULT_RCVTIMEO -1
#define NML_DEFAULT_SNDTIMEO -1
#define NML_DEFAULT_RECONNECT_IVL 100
#define NML_DEFAULT_RECONNECT_IVL_MAX 0
#define NML_DEFAULT_SNDPRIO 8
#define NML_DEFUALT_IPV4_ONLY 1
#define NML_DEFAULT_BUFFER (128 * 1024)

#define get_socket(L) (nml_socket_ud *) luaL_checkudata(L, 1,NML_SOCKET_TYPE)
#define nml_error(L, msg) luaL_error(L, "%s:\n\t%s", msg, nn_strerror(nn_errno()));

extern const struct luaL_Reg nml_socket_methods [];

extern const int  NML_NN_DOMAINS[];
extern const char NML_DOMAIN_STRINGS[][16];
extern const int NML_NN_DOMAINS_COUNT;
extern const int  NML_NN_PROTOCOLS [];
extern const  char NML_PROTOCOL_STRINGS[][16];
extern const int NML_PROTOCOL_COUNT;

typedef struct nml_socket_ud {
	int socket;
	char * buf;
	long int len;
	lua_State *L;
} nml_socket_ud;




// enum NML_SOCKETS {NML_PAIR, NML_PUB, NML_SUB, NML_REQ, 
// 	NML_REP, NML_PUSH, NML_PULL, NML_SURVEYOR, NML_RESPONDENT, NML_BUS};






/*
static int nml_report_error(lua_State * L, const char * msg){
	return luaL_error(L, "%s:\n\t%s", msg, nn_strerror(nn_errno()));
}
*/

