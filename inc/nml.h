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

//#define CONTROL

#include <Windows.h> // needed for dllmain, and sleep
#include "stdlib.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "tchar.h"
#include "nn.h"
#include "chunk.h"
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

#define nml_api(a) \
	static const TCHAR g_ach##a[] = _T(#a);\
	int l_##a(lua_State* L);

nml_api(FD_CLR)
nml_api(FD_ISSET)
nml_api(FD_SET)
nml_api(FD_ZERO)
nml_api(select)
nml_api(sleep)
nml_api(socket)
nml_api(close)
nml_api(setsockopt)
nml_api(getsockopt)
nml_api(bind)
nml_api(connect)
nml_api(shutdown)
nml_api(send)
nml_api(recv)
nml_api(sendmsg)
nml_api(recvmsg)
nml_api(allocmsg)
nml_api(freemsg)
nml_api(cmsg)
nml_api(poll)
nml_api(errno)
nml_api(strerror)
nml_api(symbol)
nml_api(symbolinfo)
nml_api(device)
nml_api(term)
nml_api(env)
nml_api(nml_msg)
nml_api(msg_alloc)
nml_api(msg_realloc)
nml_api(msg_free)
nml_api(msg_getbuffer)
nml_api(msg_getheader)
nml_api(msg_setheader)
nml_api(msg_fromstring)
nml_api(msg_tostring)
nml_api(msg_getsize)

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
	{g_achFD_CLR, l_FD_CLR}, {g_achFD_ISSET, l_FD_ISSET}, {g_achFD_SET, l_FD_SET}, {g_achFD_ZERO, l_FD_ZERO},
	{g_achnml_msg, l_nml_msg}, {g_achmsg_alloc, l_msg_alloc}, {g_achmsg_realloc, l_msg_realloc}, {g_achmsg_free, l_msg_free}, {g_achmsg_getbuffer, l_msg_getbuffer}, 
	{g_achmsg_getheader, l_msg_getheader}, {g_achmsg_setheader, l_msg_setheader}, {g_achmsg_fromstring, l_msg_fromstring}, {g_achmsg_tostring, l_msg_tostring}, 
	{g_achmsg_tostring, l_msg_tostring}, {g_achmsg_getsize, l_msg_getsize}
};
static const int g_inmlApis = sizeof(g_apchApi)/sizeof(g_apchApi[0]);

//////////////////////////////////////////////////////////////////////////
// private functions

int dump_stack(lua_State *L, const char * msg);

// chunk manipulation
void* ck_alloc(const int in_i);
int ck_free(void* in_pv);
void* ck_realloc(void* in_pv, const int in_i);
void* ck_get_data(void* in_pvck);
void ck_set_header(void* in_pvck, const DWORD in_dw);
void ck_copy_data(void* io_pvck, const void* in_pvsrc, const size_t in_iSize);
const char* ck_get_header(void* in_pvck);
int ck_get_size(void* in_pvck);