/*
    Copyright (c) 2014 Tightrope Media Systems inc.  All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#pragma once

#include <Windows.h> // needed for dllmain, and sleep
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

//max string length for options
#define NML_MAX_STR 1024

static const TCHAR g_achFD_CLR[] = _T("FD_CLR");
int l_FD_CLR(lua_State* L);

static const TCHAR g_achFD_ISSET[] = _T("FD_ISSET");
int l_FD_ISSET(lua_State* L);

static const TCHAR g_achFD_SET[] = _T("FD_SET");
int l_FD_SET(lua_State* L);

static const TCHAR g_achFD_ZERO[] = _T("FD_ZERO");
int l_FD_ZERO(lua_State* L);

static const TCHAR g_achselect[] = _T("select");
int l_select(lua_State* L);

static const TCHAR g_achsleep[] = _T("sleep");
int l_sleep(lua_State* L);

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

typedef struct nml_msg { 
	void * message;
	size_t size; 
} nml_msg;

//////////////////////////////////////////////////////////////////////////

typedef struct SnmlApi{
	const TCHAR *name;
	lua_CFunction fn;
}SnmlApi;

static struct SnmlApi g_apchApi[] = {{g_achsocket, l_socket}, {g_achclose, l_close}, {g_achsetsockopt, l_setsockopt},
	{g_achgetsockopt, l_getsockopt}, {g_achbind, l_bind}, {g_achconnect, l_connect}, {g_achshutdown, l_shutdown}, {g_achsend, l_send}, 
	{g_achrecv, l_recv}, {g_achsendmsg, l_sendmsg}, {g_achrecvmsg, l_recvmsg}, {g_achallocmsg, l_allocmsg}, {g_achfreemsg, l_freemsg}, 
	{g_achcmsg, l_cmsg}, {g_achpoll, l_poll}, {g_acherrno, l_errno}, {g_achstrerror, l_strerror}, {g_achsymbol, l_symbol}, {g_achsymbolinfo, l_symbolinfo},
	{g_achdevice, l_device}, {g_achterm, l_term}, {g_achsleep, l_sleep}, {g_achselect, l_select}, 
	{g_achFD_CLR, l_FD_CLR}, {g_achFD_ISSET, l_FD_ISSET}, {g_achFD_SET, l_FD_SET}, {g_achFD_ZERO, l_FD_ZERO}};
static const int g_inmlApis = sizeof(g_apchApi)/sizeof(g_apchApi[0]);

int dump_stack(lua_State *L, const char * msg);