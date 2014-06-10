#pragma once

//#include <Windows.h> // needed for dllmain
#include "stdlib.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "tchar.h"
#include "nn.h"
#include "pipeline.h"
#include "pubsub.h"
#include "reqrep.h"
#include "bus.h"
#include "pair.h"
#include "survey.h"
#include "assert.h"
#include "tcp.h"
#include "ipc.h"
#include "inproc.h"

// parameter offset on the lua stack
// I am using this define since I'm not 100% positive yet if I'll do nml:socket or nml.socket.
#define P1 1
#define P2 2
#define P3 3
#define P4 4
#define P5 5

static const TCHAR g_achInvalidSocketParameter[] = _T("expected first parameter to be an integer (socket identifier)");

static const TCHAR g_achsocket[] = _T("socket");
int l_socket(lua_State* L);

static const TCHAR g_achclose[] = _T("close");
int l_close(lua_State* L);

static const TCHAR g_achsetsockopt[] = _T("setsockopt");
int l_setsockopt(lua_State* L);

static const TCHAR g_achgetsockopt[] = _T("getsockopt");
int l_getsockopt(lua_State* L);

static const TCHAR g_achbind[] = _T("bind");
int l_bind(lua_State* L);

static const TCHAR g_achconnect[] = _T("connect");
int l_connect(lua_State* L);

static const TCHAR g_achshutdown[] = _T("shutdown");
int l_shutdown(lua_State* L);

static const TCHAR g_achsend[] = _T("send");
int l_send(lua_State* L);

static const TCHAR g_achrecv[] = _T("recv");
int l_recv(lua_State* L);

static const TCHAR g_achsendmsg[] = _T("sendmsg");
int l_sendmsg(lua_State* L);

static const TCHAR g_achrecvmsg[] = _T("recvmsg");
int l_recvmsg(lua_State* L);

static const TCHAR g_achallocmsg[] = _T("allocmsg");
int l_allocmsg(lua_State* L);

static const TCHAR g_achfreemsg[] = _T("freemsg");
int l_freemsg(lua_State* L);

static const TCHAR g_achcmsg[] = _T("cmsg");
int l_cmsg(lua_State* L);

static const TCHAR g_achpoll[] = _T("poll");
int l_poll(lua_State* L);

static const TCHAR g_acherrno[] = _T("errno");
int l_errno(lua_State* L);

static const TCHAR g_achstrerror[] = _T("strerror");
int l_strerror(lua_State* L);

static const TCHAR g_achsymbol[] = _T("symbol");
int l_symbol(lua_State* L);

static const TCHAR g_achsymbolinfo[] = _T("symbolinfo");
int l_symbolinfo(lua_State* L);

static const TCHAR g_achdevice[] = _T("device");
int l_device(lua_State* L);

static const TCHAR g_achterm[] = _T("term");
int l_term(lua_State* L);

static const TCHAR g_achenv[] = _T("env");
int l_env(lua_State* L);

//////////////////////////////////////////////////////////////////////////

typedef struct SnmlApi{
	const TCHAR *name;
	lua_CFunction fn;
}SnmlApi;

static struct SnmlApi g_apchApi[] = {{g_achsocket, l_socket}, {g_achclose, l_close}, {g_achsetsockopt, l_setsockopt},
	{g_achgetsockopt, l_getsockopt}, {g_achbind, l_bind}, {g_achconnect, l_connect}, {g_achshutdown, l_shutdown}, {g_achsend, l_send}, 
	{g_achrecv, l_recv}, {g_achsendmsg, l_sendmsg}, {g_achrecvmsg, l_recvmsg}, {g_achallocmsg, l_allocmsg}, {g_achfreemsg, l_freemsg}, 
	{g_achcmsg, l_cmsg}, {g_achpoll, l_poll}, {g_acherrno, l_errno}, {g_achstrerror, l_strerror}, {g_achsymbol, l_symbol}, {g_achsymbolinfo, l_symbolinfo},
	{g_achdevice, l_device}, {g_achterm, l_term}};
static const int g_inmlApis = sizeof(g_apchApi)/sizeof(g_apchApi[0]);

//////////////////////////////////////////////////////////////////////////

typedef struct SNameValue{
	TCHAR* name;
	int value;
}SNameValue;

struct SNameValue* g_NanomsgProtocols; /*[] = {
	{_T("NN_PUSH"), NN_PUSH}, 
	{_T("NN_PULL"), NN_PULL}, 
	{_T("NN_PUB"), NN_PUB}, 
	{_T("NN_SUB"), NN_SUB}, 
	{_T("NN_REQ"), NN_REQ}, 
	{_T("NN_REP"), NN_REP}, 
	{_T("NN_BUS"), NN_BUS}, 
	{_T("NN_PAIR"), NN_PAIR}, 
	{_T("NN_SURVEYOR"), NN_SURVEYOR}, 
	{_T("NN_RESPONDENT"), NN_RESPONDENT}};*/
int g_iNanoMsgProtocols; //= sizeof(g_NanomsgProtocols)/sizeof(g_NanomsgProtocols[0]);

//////////////////////////////////////////////////////////////////////////

static struct SNameValue g_NanomsgLevels[] = {
	{_T("NN_SOL_SOCKET"), NN_SOL_SOCKET}, 
	{_T("NN_PUSH"), NN_PUSH}, 
	{_T("NN_PULL"), NN_PULL}, 
	{_T("NN_PUB"), NN_PUB}, 
	{_T("NN_SUB"), NN_SUB}, 
	{_T("NN_REQ"), NN_REQ}, 
	{_T("NN_REP"), NN_REP}, 
	{_T("NN_BUS"), NN_BUS}, 
	{_T("NN_PAIR"), NN_PAIR}, 
	{_T("NN_SURVEYOR"), NN_SURVEYOR}, 
	{_T("NN_RESPONDENT"), NN_RESPONDENT},
	{_T("NN_TCP"), NN_TCP},
	{_T("NN_IPC"), NN_IPC},
	{_T("NN_INPROC"), NN_INPROC}};
static const int g_iNanomsgLevels = sizeof(g_NanomsgLevels)/sizeof(g_NanomsgLevels[0]);

//////////////////////////////////////////////////////////////////////////

static struct SNameValue g_NanomsgOptionTypes[] = {
	{_T("NN_LINGER"), NN_LINGER},
	{_T("NN_SNDBUF"), NN_SNDBUF},
	{_T("NN_RCVBUF"), NN_RCVBUF},
	{_T("NN_SNDTIMEO"), NN_SNDTIMEO},
	{_T("NN_RCVTIMEO"), NN_RCVTIMEO},
	{_T("NN_RECONNECT_IVL"), NN_RECONNECT_IVL},
	{_T("NN_RECONNECT_IVL_MAX"), NN_RECONNECT_IVL_MAX},
	{_T("NN_SNDPRIO"), NN_SNDPRIO},
	{_T("NN_IPV4ONLY"), NN_IPV4ONLY},
	{_T("NN_SOCKET_NAME"), NN_SOCKET_NAME},
};
static const int g_iNanomsgOptionTypes = sizeof(g_NanomsgOptionTypes)/sizeof(g_NanomsgOptionTypes[0]);

//////////////////////////////////////////////////////////////////////////

int dump_stack(lua_State *L, const char * msg);