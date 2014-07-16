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
A message is a table that contains a userdata pointing to a custom-allocated buffer, usually allocated by NML.
The message's table is populated with functions. These functions allow message payload allocation and message content access.
The NML.Lua layer will populate the message table functions, thus determining which allocators will be used.

A nml message table should expose the following functions (using nml's implementations in this example):

    msg.alloc = nml.msg_alloc
    msg.free = nml.msg_free
    msg.realloc = nml.msg_realloc
    msg.getbuffer = nml.msg_getbuffer
    msg.getallocatortype = function() ... return the allocator string... end

The above are used by C modules to access the message's payload.

###3.2 C representation
If NML is used to allocate functions, then the allocator type should be set to "nml" (not case sensitive).
The preferred allocator type is "NML ". Using this allocator will guarantee zero-copy on the nanomsg side.
If the allocator type is set to "NML " some assumptions will be made in NML.core about the pvBuffer content (handled as a RIFF, see below). The buffer data will be accessed directly and not through the lua functions.

NML's C core will access the message through its getbuffer api. Again if NML is the allocator then it will directly call the assigned C functions without using lua call.

#####nml.send
If the allocator type is set to "nml" then nml.core will use nanomsg's zero-copy mechanism.

The allocator can also be set to a custom type, in which case NML.core will access the buffer's data through the buffer's Lua functions as found in the buffer's metatable. This will cause the buffer to be copied into a nanomsg-allocated buffer.

#####nml.recv
Receives a message from an SP socket and generates a new nml message out of it.
    
    local my_message = nml.recv()

The incoming data is always allocated by nanomsg. 
nml.core sets the message's table to use nml functions.
nml.core sets the allocator type to "nml" upon receiving new data.

###3.4 NML payload allocation
The buffer data is represented using a RIFF formatting. The first 4 bytes are the FCC code, followed by 4 bytes containing the size of the chunk (excluding the FCC and size identifiers), and followed by the chunk itself. 

The payload chunk may be followed by a junk chunk to satisfy alignment requirements.

The FCC code is used to identify the payload type (ex: MSGP for MessagePack). 
FCC codes used are in the context of MediaCircus, and may clash with registered FCC codes.
Note that the FCC code only allows four characters, which differs from the allocator, represented by a lua string.

**The FCC code allows identification of the data type on the receiving end.

**The RIFF format is what gets sent over the SP socket, regardless of the allocator type.

    0       4       8                                                         104     108     112       128
    +-------+-------+----------------------------------------------------------+-------+-------+---------+ 
    |       |       |                                                          |       |       |         | 
    |   M   |       |                                                          |   J   |       |         | 
    |   S   | 0x0060|                                                          |   U   | 0x0010| YYYYYYY | 
    |   G   |       |                         data                             |   N   |       |         | 
    |   P   |       |                                                          |   K   |       |         | 
    |       |       |                                                          |       |       |         | 
    +-------+-------+----------------------------------------------------------+-------+-------+---------+ 

The RIFF chunk details are hidden from the lua view. alloc, free, realloc all operate on the data portion of the buffer. 
The nml.msg_* functions will handle RIFF parsing internally. For example, calling nml.msg_alloc will allocate the first 8 bytes, plus the requested size, plus the trailing junk chunk if applicable. It will then return a pointer to the data chunk.

The riff chunk format contains all information needed to parse the buffer. Size, type and data.

We'll support allocating the data type before knowing the chunk fourCC code. 
The fourCC code is considered the 'header' and specifies the data format, and is set by the lua layer through the buffer api.

---

#4 NML api
Nml.core implements the following functions to access the buffer's userdata object.
They allow manipulation of a nml message userdata.

  msg_alloc
  msg_realloc
  msg_free
  msg_getheader
  msg_fromstring
  msg_getbuffer
  msg_frombuffer
  msg_tostring
  msg_getsize
  
Note: a function callback made from one C module can be executed in another C module, using the Lua stack as a middle man:
                                                                                        
                               +-------+                                             
                               |       |                                             
                               |  Lua  |                                             
                  +----------> |       +-----------+                                 
                  |            +-------+           |                                 
                  |                                |                             LUA 
                  |                                |                                 
   +--------------------------------------------------------------------------------+
                  |                                |                                 
1.call(msg_alloc) |                                v                              C  
                  |                                    2.nn_allocmsg                 
           +------+-------+                    +-------+         +-----------+       
           |              |                    |       |         |           |       
           | MessagePack  |                    |  NML  +-------> |  NanoMsg  |       
           |              |                    |       |         |           |       
           +--------------+                    +-------+         +-----------+       

###workflow example
Send example:

    -- this is our message table
    local my_msg = {
      alloc = nml.msg_alloc,
      realloc = nml.msg_realloc,
      free = nml.msg_free,
      getheader = nml.msg_getheader,
      getsize = nml.msg_getsize,
      getbuffer = nml.msg_getbuffer, -- return the payload section of the buffer ud riff
    }

    -- we're using nml allocators, return the string 'nml'
    my_msg.getaallocatortype = function return "nml" end

    -- return the buffer payload if allocated, nil otherwise
    my_msg.getbuffer = function return self.buffer end

    -- NOTE: don't assign __gc to msg_free, since nml.send will free the message payload

    -- populate the message payload
    my_msg.buffer = nml.msg_fromstring("Hello World!")
    -- note: when using fromstring, getallocatortype needs to return "nml" whenever asked

    -- the data is populated
    assert(type(my_msg.buffer)=="userdata")

    -- send it over the SP socket
    -- this will free the message's nml-allocated buffer
    nml.send(my_msg)

    -- remove the buffer, it's been consumed
    my_msg.buffer = nil

Receive example:

    local my_msg = nml.recv()

    -- consume the message
    print("received "..nml.msg_tostring(my_msg))

    -- free the message
    nml.msg_free(my_msg) -- when receiving data, this could be put in the __gc processing
    my_msg = nil

###4.1 nml.msg_fromstring(msg_string)
Creates a new message userdata and initializes it with the specified lua string content.
This will use the nml allocator to allocate the buffer userdata object.

Returns the message userdata if successful, nil and an error message string in case of error.

#####Implementation

    size_t sizeStr;
    char* pchString = lua_tolstring(L, 1, &sizeStr);
    
    // allocate the buffer that will hold the string
    lua_pushinteger(L, sizeStr);
    msg_alloc(L, sizeStr);

    char** ppchBuffer = (char**)lua_touserdata(L, -1);

    // +8 to offset the riff
    memcpy((*ppchBuffer)+8, pchString, sizeStr);

    // return the userdata buffer
    return 1;

---

###4.2 nml.msg_getsize(buffer_ud)
Gets the buffer's size by accessing the riff's size identifier.

Returns the size integer if successful, nil and an error message string in case of error.

---

###4.3 nml.msg_alloc(size)
Allocates a userdata that will point to the allocated memory.
The allocated memory will be of the specified size.
Calls nn_allocmsg to allocate the memory.

Returns a userdata if successful, or nil and an error message string in case of error.

**Note: the caller needs to return "nml" when asked for the allocator type (getallocatortype).

#####Implementation

    void** ppvBuffer = (void**)lua_newuserdata(L, sizeof(void*));
    **ppvBuffer = nn_allocmsg(size+8); // the ckid and cksize overhead
    return 1;

---

###4.4 nml.msg_realloc(buffer_ud, size)
Reallocates a buffer of the specified size.
Calls nn_reallocmsg with buffer_ud.

Returns a userdata if successful, or nil and an error message string in case of error.

---

###4.5 nml.msg_free(buffer_ud)
Frees the memory pointed to by buffer_ud, and sets the buffer_ud to nil.

Returns true if it freed the message, false if buffer_ud is NULL. Returns nil and an error message string in case of error.

---

###4.6 nml.msg_setheader(msg_ud, header_str)
Sets the message userdata riff's header chunk to the specified fourCC code.
If the header_str is longer than four characters it will be truncated.

---

###4.7 nml.msg_getheader(msg_ud)
Returns the message's fourCC code as a lua string.

---

###4.8 nml.send
This is not a message API, but it will need to be modified to support the Message userdata.
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

###4.9 nml.recv
This is not a message API, but it will need to be modified to support the Message userdata.
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

---

4.10 nml.msg_tostring
Copies the content of pvBuffer into a lua string.

Always returns a string. If pvBuffer is NULL then it will return an empty string.

---

4.11 nml.msg_frommessage
Clones the specified message into a new message user data.

    nml.msg_frommessage(source_msg_ud, dest_msg_ud)

    assert(#new_msg_ud==#source_msg_ud)

---

4.12 nml.msg_getbuffer
Returns the payload section of the buffer userdata's riff.
Used by C modules to access the raw data without needing to know about its implementation details.

---

#5 Using MessagePack as a serializer
MessagePack will be made into a NuGet that can be imported by NML and any other modules requiring lua C MessagePack support. 

MessagePack's C core will access the memory allocators through the message's alloc, realloc and free api.

###5.1 Implementation details
We need to modify the C module to use the Message's memory allocator callbacks in its pack function. 
The unpack function will use a regular malloc. In this case the message metatable's __gc will be implemented to call free.

Add function pointers for alloc, realloc and free in the mp_buf structure. 
Whenever a malloc, realloc or free is called on the mp_buf payload, replace with the corresponding mp_buf function.

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

---

#6 generic binary serialization
This pertains to the issue of non-NML userdata, and to how this kind of userdata can be serialized when nested inside tables. 
A good example of this is a DSX buffer representing picture data. We need the ability to transfer this userdata over NML, without requiring DSX to know anything about how a NML message should be formatted.

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

3. Implement all nml functions.
4. test against item 2.
--> M2: nml message api completed.

5. Implement nml MessagePack.
6. test against item 1.
--> M3: MessagePack done.

7. Write a complete lua test script to test all functionalities.
--> M4: all done