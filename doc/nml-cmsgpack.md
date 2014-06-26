#1. NML MessagePack
This document outlines the details of NML's MessagePack support.

###1.1 Goal
Provide an efficient, binary-compatible, seamless and extensible encoding and decoding mechanism as part of NML.

###1.2 Current problem
The current messaging implementation relies on the MediaCircus/application layer to transform a message into a string. 

This approach doesn't support plain old data (POD) buffers, typically represented by userdata or lightuserdata types.

It is also not the most efficient, as the buffer needs to be copied on every step. 

Here is a typical send operation using a JSON serializer (could be a pl.pretty.write, or some other mechanism):

+---------------+    1.{lua_table}    +---------------+                                 
|               +-------------------> |               |                                 
|               |                     |    JSON       |                                 
|  Application  |                     |     Lua       |                                 
|      Lua      |   2."[{lua_tablee}]"|               |                                 
|               | <-------------------+               |                                 
+-------+-------+                     +---------------+                                 
        |                                                                   socket      
        |                                                                      ^        
        | 3."[{lua_tablee}]"                                                   |        
        v                                                                      |        
                                                                               |        
+---------------+                  +---------------+                   +-------+-------+
|               |                  |               |                   |               |
|               | 4."[{lua_table}]"|               | 5."[{lua_table}]" |    nanomsg    |
|    NML Lua    +----------------> |    NML C      +-----------------> |     (dll)     |
|               |                  |               |                   |               |
|               |                  |               |                   |               |
+---------------+                  +---------------+                   +---------------+

In this example my_big_lua_table is a lua table. JSON:encode() is called which formats the table into a JSON string. NML:send() is called with the JSON string, which is then copied into a nanomsg buffer and sent over the socket. The data is thus represented 3 times in memory.

The receive operation has a similar workflow:

    socket --> nanomsg:["{my_big_lua_table}"] --> LUA:"{my_big_lua_table}" --> JSON:{my_big_lua_table}

In this case nanomsg allocates a buffer to receive the incoming data. It then pushes the result to lua using pushlstring. JSON is then called to rebuild the original table.

---

#2. solutions
Use a serializer as an independent C module with a lua binding. It will be made into a NuGet that can be imported by NML and any other modules.

###2.1 Embed the serializer into NML through NML's lua layer, and provide nanomsg's C low level memory management callbacks to the serializer.

Note: Application = MediaCircus filter (typically)

                                                                        message                                                             
+---------------+                     +---------------+ 3.{lua_table},(empty)       +---------------+                    +---------------+
|               |                     |               +---------------------------> |               |   message.alloc,   |               |
|               |    1.{lua_table}    |               |                             |  Serializer   | 4.message.realloc  |  Serializer   |
|  Application  +-------------------> |    NML Lua    |          message            |     Lua       | <----------------> |       C       |
|      Lua      |                     |               |        5.(filled)           |               |                    |     (dll)     |
|               |                     |               | <---------------------------+               |                    |               |
+---------------+                     +-------------+-+                             +---------------+                    +---------------+
                                                    |                                                                                     
                                        ^           |   message                                                                           
                                        |           | 6.(filled)                                                                          
                              2.message |           |                                                                                     
                               (empty)  |           v                                                                                     
                                        |                                                                                                 
                                      +-+-------------+   message.pbuffer, +---------------+                                              
                                      |               |   message.len,     |               |                                              
                                      |               | 7.message.header   |    nanomsg    |                                              
                                      |    NML C      +------------------> |     (dll)     |                                              
                                      |    (dll)      |                    |               |                                              
                                      |               |                    |               |                                              
                                      +---------------+                    +-------+-------+                                              
                                                                                   |                                                      
                                                                                   |                                                      
                                                                                   v                                                      
                                                                                 socket                                                   
---

#3 Message format
##3.1 Lua representation
A message is a userdata allocated by NML.
The userdata has a metatable also created and assigned by NML. 

The NML.Lua layer can populate the message's metatable.
*(to be confirmed) NML does not define the __newindex function on the message userdata's metatable. So it must create a placeholder for all exported msg functions.

A Message's metatable exposes the following API:

    {
      __gc,
      __len,
      alloc,      // allows a 3rd party C module to populate the pvBuffer
      realloc,    // ""
      free,       // allows a 3rd party C module to manage the pvBuffer (overwrite it by first deleting it...)
      getbuffer,  // allows a 3rd oarty C module to retrieve pvBuffer
      getheader,  // allows a 3rd party C module to retrieve pvHeader
    }

The default implementation uses the following callbacks:

    msg.alloc = nml.msg_alloc
    msg.free = nml.msg_free
    msg.realloc = nml.msg_realloc
    getmetatable(msg).__len = nml.msg_getsize
    msg.getbuffer = nml.msg_getbuffer
    msg.getheader = nml.msg_getheader

When NML allocates a msg_ud it will call the proper internal functions and populate the msg_ud metatable with the appropriate memory allocation functions. A default implementation for each of the metatable functions is provided by NML (api contract is guaranteed).

###3.2 C representation

    struct SNmlMessage {
      void* pvBuffer;
      uint64_t ui64BufferSize;
      void* pvHeader;
      uint64_t ui64HeaderSize;
    };

The header is propagated to lua as a string type, but internally is handled similarly to pvBuffer. There are no restrictions on the pvHeader's payload. It is only constrained by the underlying transport module (nanomsg).

---

#4 NML api
Note: It's interesting to appreciate here that a function callback made from one C module will be executed in another C module, using the Lua stack as a middle man.

###4.1 nml.new_msg()
Creates a new Message userdata object. Specifies a type name common to all messages, in order to later use luaL_checkudata(..., name).

Returns a new message userdata if successful, nil and an error message string in case of error.

---

###4.2 nml.msg_fromstring(msg_ud, msg_string)
Copies msg_string's content into SNmlMessage.pvBuffer. 
Uses lua_tolstring to determine the size of the copied buffer, sets SNmlMessage.ui64Size.

Returns the message userdata if successful, nil and an error message string in case of error.

---

###4.3 nml.msg_getsize(msg_ud)
Gets SNmlMessage.ui64Size, the size of the data pointed to by SNmlMessage.pvBuffer.

Returns the size if successful, nil and an error message string in case of error.

#####Implementation

    SNmlMessage* pMessage = luaL_checkudata(L, -1, metatable_name_str);
    lua_pushinteger(L, pMessage->ui64Size);
    return 1;

---

###4.4 nml.msg_getbuffer
Returns the Message's buffer (as a lightuserdata) and its size. This is typically used by 3rd party C modules to access the payload without needing to case the msg_ud into a C struct.

    SNmlMessage* pMessage = luaL_checkudata(L, 1, metatable_name_str);
    lua_pushlightuserdata(L, pMessage->pvBuffer);
    lua_pushinteger(L, pMessage->ui64BufferSize);
    return 2;

---

###4.5 nml.msg_alloc(msg_ud, size)
Allocates a buffer of the specified size in SNmlMessage.
Calls nn_allocmsg, and sets SNmlMessage.pvBuffer.
Sets SNmlMessage.ui64Size.

Returns Message userdata if successful, or nil and an error message string in case of error.

---

###4.6 nml.msg_realloc(msg_ud, size)
Reallocates a buffer of the specified size in SNmlMessage.
Calls nn_reallocmsg, and sets SNmlMessage.pvBuffer.
Sets SNmlMessage.ui64Size.

Returns Message userdata if successful, or nil and an error message string in case of error.

---

###4.7 nml.msg_free(msg_ud)
Frees the SNmlMessage.pvBuffer memory, and sets SNmlMessage.ui64Size to 0.

Returns true if it freed the message, false if SNmlMessage.pvBuffer is NULL. Returns nil and an error message string in case of error.

#####Implementation

    SNmlMessage* pMessage = luaL_checkudata(L, -1, metatable_name_str);
    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "free");
    lua_pushlightuserdata(L, pMessage->pvBuffer);
    lua_call(L, 1, 0); // calls nn_freemsg in this example

    // zero out the pointer and its size
    pMessage->pvBuffer = NULL;
    pMessage->ui64Size = 0;
    
    // return the ud
    lua_settop(L, 1);
    return 1;

---

###4.8 nml.msg_setheader(msg_ud, header_str)
Allocates a header buffer, and puts the specified header string into the message's header buffer. Sets the message's header size to match the lua string size, not adding the terminator.

    SNmlMessage* pMessage = luaL_checkudata(L, -1, metatable_name_str);
    size_t sizeHeader;
    void* pvHeader = luaL_checklstring(L, 2, &sizeHeader);

    // Calls nn_allocmsg, and sets SNmlMessage.pvBuffer.
    pMessage->pvHeader = nn_allocmsg(sizeHeader);

    memcpy(pMessage->pvHeader, pvHeader, sizeHeader);
    pMessage->ui64HeaderSize = sizeHeader;

    // return the msg_ud
    lua_settop(L, 1);
    return 1;

---

###4.9 nml.msg_getheader(msg_ud)
Returns the message's header as a lua string.

    SNmlMessage* pMessage = luaL_checkudata(L, 1);

    if (pMessage->pvHeader!=NULL) {
      lua_pushlstring(L, pMessage->pvHeader, pMessage->ui64HeaderSize);
      return 1;
    }

---

###4.10 nml.send
Sends a Message to an SP socket.

    nml.send(socket, msg_ud, flags)

#####parameters
- socket (number): the SP socket
- msg_ud (userdata): the Message to send
- flags (number): set to NN_DONTWAIT if non-blocking, or 0.

#####return values
success:
- [1]: (number) 0

error: 
- [1]: nil
- [2]: (string) a helpful description of the error

#####remarks
The msg_ud's buffer and header memory will be freed and set to NULL by the send.

---

###4.11 nml.recv
Receives a message coming from an SP socket and pushes it to lua as a string buffer.

     local msg_ud = nml.recv(socket, flags)

#####parameters
- socket (number): the SP socket.
- flags (number): set to NN_DONTWAIT if non-blocking, or 0.

#####return values
success:
- [1]: (userdata) the Message

error:
- [1]: nil
- [2]: (string) a helpful description of the error

#####Implementation

    // in[1]: socket
    // in[2]: flags

    nn_msghdr hdr;
    SNmlMessage* pMessage;
    struct nn_iovec iov;
    struct nn_msghdr hdr;

    // create a new message
    new_msg(L);
    pMessage = luaL_checkudata(L, -1, message_metatable_str);

    // put it at L[3]
    lua_insert(L, 3);

    iov.iov_base = pMessage->pvBuffer;
    iov.iov_len = NN_MSG;

    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;

    hdr.msg_control = pMessage->pvHeader;
    hdr.msg_controllen = NN_MSG;

    pMessage->ui64BufferSize = nn_recv(socket, &hdr, flags);

    // retrieve the header size
    pMessage->ui64HeaderSize = nn_chunk_size(pMessage->pvHeader);

    // return the msg_ud
    lua_pushvalue(L, 3);
    return 1;

---

#5 Using MessagePack as a serializer
MessagePack will be made into a NuGet that can be imported by NML and any other modules requiring lua C MessagePack support. 

MessagePack's C core will access the memory allocators through the message's alloc, realloc and free api.

###5.1 Implementation details
We need to modify the C module to use the Message's memory allocator callbacks in its pack and unpack methods.

Add a lua_State* parameter to mp_buf_new and mp_cur_new, as well as functions that call free and realloc.

Wrap the memory allocator functions and branch code to the desired allocation mechanism (default malloc/free vs message.alloc/free etc.).    

###5.2 Notes on MessagePack

There are some important aspects of MessagePack that we should be aware of.

- I am currently basing my MessagePack assumptions on this binding: https://github.com/antirez/lua-cmsgpack
- that MessagePack binding uses a BSD two clause license, I will probably need to update this project's license (MIT)
- **MessagePack is Big Endian**, but I'm copying the POD buffers using memcpy, not swapping the byte order since all of our MC components are little endian at the moment.
- MessagePack encoding and decoding are two step processes. They both allocate a temporary internal buffer to store the encoded data, which is then pushed to lua as a string buffer.
- MessagePack will need to be updated to handle our way of serializing binary data (lua userdata).

---

###5.3 messagepack.pack({table_to_pack}, msg_ud)
Packs the specified table into the msg_ud.
Pass the Message's userdata to the pack call. At this point the message should be empty. 
If the message is not empty, then the message's content will be freed using the Message's own free function.
Access the Message's memory by using the Message's lua metadata functions.

    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "alloc");
    lua_pushvalue(L, 1); // the ud
    lua_pushinteger(L, 1024); // pretend it needs a 1k buffer

    // allocate the buffer
    lua_call(L, 2, LUA_MULTRET); // the ud and its size
    
    // get the pbuf
    lua_settop(L, 2); // msg_ud, mt
    lua_getfield(L, 2, "getbuffer");
    void* pvBuffer = lua_touserdata(L, -1);

    // fill the pbuf
    memcpy(pvBuffer, ...);

    ...

    // return the Message
    lua_settop(L, 1);
    return 1;

---

###5.4 MessagePack.unpack(msg_ud)
Unpacks the msg_ud into a table and returns the table. 
Does not free msg_ud.

    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "getbuffer");
    lua_call(L, 1, LUA_MULTRET);

    void* pvBuffer = lua_touserdata(L, -1);

    // unpack pvBuffer

    // return the table
    return 1;

---

#6 generic binary serialization
This pertains to the issue of non-NML userdata, and to how this kind of userdata can be serialized when nested inside tables. 
A good example of this is a DSX buffer representing picture data. We need the ability to transfer this userdata over NML, without requiring DSX to publish it using a NML formatting (SNmlMessage).

We'll implement our own custom payload package:

    { 
      buffer_ud = <some_userdata_value>, 
      buffer_ud_type = "dsx_filter_buffer", 
      buffer_ud_size = <the_size>,
      buffer_ud_allocator = "nml_msg",
    }

This binary serialization representation will be universal across all MC modules.

If MessagePack encounters a nested userdata type, it should look for the type, size and allocator fields. If it finds all fields then it can go ahead and serialize it in its packed buffer, and maintain the table descriptor fields. It should set the buffer_ud_allocator to "MessagePack".

I'll update existing DSX modules to reflect this formatting.

---

#7. work breakdown

1. Write serialization lua tests for cmsgpack.
2. Write nml.msg* api lua tests.
--> M1: lua tests code complete.

3. Implement nml message functions.
4. test against item 2.
--> M2: nml message api completed.

5. Implement nml MessagePack.
6. test against item 1.
--> M3: MessagePack done.

7. Write a complete lua test script to test all functionalities.
--> M4: all done