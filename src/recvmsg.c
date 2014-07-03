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

#include "nml.h"
#define NN_MSG_CTRL_SIZE 256
// fine-grained alternative to nn_recv
// http://nanomsg.org/v0.3/nn_recvmsg.3.html
int l_recvmsg(lua_State* L)
{
	// receive data using a nn-allocated buffer
	// push the received data to a lua string (new buffer) -- we expect to always receive strings
	// free the nn-alloc'd buffer
	
	struct nn_msghdr msg;
	struct nn_iovec iov;
	void * msg_buff;
	void * msg_ctrl;
	int s, flags, recvd_bytes;

	//header vars
	struct nn_cmsghdr * hdr;
	size_t len;
	unsigned char * data;
	
	
	dump_stack(L, "here!\n\n");

	msg_buff = msg_ctrl = 0;
	memset (&msg, 0, sizeof (msg));

	iov.iov_base = &msg_buff;
	iov.iov_len = NN_MSG;
	msg.msg_control = &msg_ctrl;
	msg.msg_controllen = NN_MSG;
	//hdr.msg_control = NULL;

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	
	
	
	//get the socket and flags from Lua
	s = luaL_checkint(L, P1);
	//flags = luaL_optint(L, P2, 0);
	flags = 0;
	//do the work
	
	recvd_bytes = nn_recvmsg (s, &msg, 0);


	if (recvd_bytes !=-1) {
		hdr = NN_CMSG_FIRSTHDR (&msg);
		if (hdr != NULL) {
			len = hdr->cmsg_len - sizeof (struct nn_cmsghdr);
			 printf ("level: %d property: %d length: %dB, space: %d, len: %d\n\n",
				  (int) hdr->cmsg_level,
				  (int) hdr->cmsg_type,
				  (int) len,
				  (int) NN_CMSG_SPACE(len),
				  (int) NN_CMSG_LEN(len)
			);
			 data = NN_CMSG_DATA(hdr);
			 //len = 6; //As reported, len is a crazy fucked up huge value (-1 == NN_MSG). So, it doesn't work.
			 //also, I don't see the data in there at all.
			 //while (len) {
				//  printf ("%c", *data);
				//  ++data;
				//  --len;
			 //}
			printf ("\n");
			hdr = NN_CMSG_NXTHDR (&msg, hdr);
		}

		printf("The header length is: %d. NN_MSG is %d. \n\n", msg.msg_controllen, NN_MSG);
		// put the string in lua space
		
		lua_pushlstring(L, (const char *) msg_buff, recvd_bytes );
		//lua_pushlstring(L, (const char *);
		// free the nn buffer
 		assert( nn_freemsg( msg_buff) != -1) ; // I don't want to return this to the caller but I should have the ability to spot this problem in debug
		assert( nn_freemsg( msg_ctrl) != -1 );
	} else
		lua_pushnil(L);
	// result
	return 1;
}