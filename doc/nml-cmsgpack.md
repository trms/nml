#1. NML MessagePack
This document outlines the details of NML's MessagePack support.

###1.1 Goal
Provide an efficient, binary-compatible, seamless and extensible encoding and decoding mechanism as part of NML.

###1.2 Current problem
The current messaging implementation relies on the MediaCircus/application layer to transform a message into a single string. This is a two step process that first uses a JSON parser to transform a lua table and its content into JSON representation. NML then sees the JSON table as a regular string type.

This approach doesn't support plain old data (POD) buffers, typically represented by userdata or lightuserdata types. Those buffers get lost in the JSON processing. 

It is also not the most efficient, as the buffer needs to be copied on every step. Here is a typical send operation:

    ------------------------------------------------------------------------------------------
    |    application    |          JSON                 |          NML                       |
    {my_big_lua_table} --> JSON:"[{my_big_lua_table}]" --> nanomsg:["[{my_big_lua_table}]"] --> socket
    |                (lua_types)                    (lua string)                             |
    ------------------------------------------------------------------------------------------
    

In this example my_big_lua_table is a lua table. JSON:encode() is called which formats the table into a JSON string. NML:send() is called with the JSON string, which is then copied into a nanomsg buffer and sent over the socket. The data is thus represented 3 times in memory.

The receive operation has a similar workflow:

    socket --> nanomsg:["{my_big_lua_table}"] --> LUA:"{my_big_lua_table}" --> JSON:{my_big_lua_table}

In this case nanomsg allocates a buffer to receive the incoming data. It then pushes the result to lua using pushlstring. JSON is then called to rebuild the original table.

###1.3 Notes on MessagePack

There are some important aspects of MessagePack that we should be aware of.

- I am currently basing my MessagePack assumptions on this binding: https://github.com/antirez/lua-cmsgpack
- that MessagePack binding uses a BSD two clause license, I will probably need to update this project's license (MIT)
- **MessagePack is Big Endian**, but I'm copying the POD buffers using memcpy, not swapping the byte order since all of our MC components are little endian at the moment.
- MessagePack encoding and decoding are two step processes. They both allocate a temporary internal buffer to store the encoded data, which is then pushed to lua as a string buffer.
- MessagePack will need to be updated to handle our way of serializing binary data (lua userdata).

#2. solutions

I propose that MessagePack remains an independent C module with a lua binding. It will be made into a NuGet that can be imported by NML and any other modules requiring lua C MessagePack support. This means that buffers still need to be exposed in lua. Possible ways to handle this situation are detailed below; 

Note: I'm going to refer to the layer on top of NML as "the application" for simplicity.

###2.1. Use MessagePack at the application level, by swapping out the JSON encoding. 
The application will transform the lua types, including userdata, through MessagePack.

    ---------------------------------------------------------------------------------------------
    |    application    |          MessagePack             |          NML                       |
    {my_big_lua_table} --> MSGPACK:"[{my_big_lua_table}]" --> nanomsg:["[{my_big_lua_table}]"] --> socket
    |               (lua types)                        (lua string)                             |
    ---------------------------------------------------------------------------------------------

Similarly to JSON, MessagePack outputs its encoded buffer as a string. The string can then be passed into NML, same as it did with JSON.

**+**

- Quick to implement.
- Delegates responsibilities to the higher level code.
- application has more control over the protocol.

**-**

- Number of times the original data is present in memory: 3+1 (MessagePack temp buffer).
- No benefits in terms of efficiency.

###2.2 Embed MessagePack into NML through NML's Lua layer, without modifying MessagePack.

Sending data: NML's entry points accept regular lua types, including userdata types. The application sends lua types directly to NML's api (numbers, strings, tables...). NML's lua layer then forwards the lua data to the desired protocol encoder (MessagePack), which encodes the lua data into a lua string buffer. NML's lua layer then passes the resulting string buffer to NML's C module.

    ---------------------------------------------------------------------------------------------
    |   application     |                          NML                                          |
    |                   |-----------------------------------                                    |
    |                   |         MessagePack              |                                    |        
    {my_big_lua_table} --> MSGPACK:"[{my_big_lua_table}]" --> nanomsg:["[{my_big_lua_table}]"] --> socket
    |               (lua types)                       (lua string)                              |
    ---------------------------------------------------------------------------------------------

**+**

- A unified, consistent way to transfer buffers between MediaCircus (MC) modules.
- Control over the protocol is tighter, and at the MC level.
- Ability to swap out the protocol if we find something better, or if the user needs another protocol (ex: protobuf)
- The MC/application no longer needs to encode the data prior to calling NML, simplifies the MC/application code.

**-**

- Number of times the original data is present in memory: 3+1.
- No benefits in terms of efficiency.

###2.3 Embed MessagePack into NML through NML's lua layer, and modify MessagePack to output a recycled userdata buffer.

This goes a step further than point 2 above. MessagePack is still embedded into NML, but it no longer copies its packed buffer into a lua string buffer, saving a buffer copy. MessagePack exports its packed buffer as userdata, which is then read directly by nanomsg. 

    --------------------------------------------------------------------------------------
    |   application     |                           NML                                  |
    |                   |---------------------------------                               |
    |                   |        MessagePack             |                               |
    {my_big_lua_table} --> MSGPACK:[{my_big_lua_table}] --> nanomsg:[{my_big_lua_table}] --> socket 
    |               (lua types)                     (lua userdata)                       |
    --------------------------------------------------------------------------------------
    
**+**

- Number of times the origina data is present in memory: 3.
- Slightly more efficient than 1 and 2.

**-**

- Need to implement a buffer management mechanism inside MessagePack's C code, to recycle the buffers safely.
- The MessagePack buffer is still copied by NML.
- MessagePack output userdata is now opaque to lua.

###2.4 Embed MessagePack into NML through NML's lua layer, and leverage NML's buffer management api.

MessagePack packs the data into a memory buffer that belongs to NML. This buffer is then sent over the SP socket through a zero-copy mechanism. 

    ------------------------------------------------------------------
    |  application      |                        NML                  |
    |                   |---------------------------------|           |
    |                   |         MessagePack             |           |     
    {my_big_lua_table} --> MSGPACK:[{my_big_lua_table}] <--> nanomsg --> socket 
    |                (lua types)                      (lua userdata)  |
    -------------------------------------------------------------------
    
**+**

- Very efficient approach, only a single copy is necessary.

**-**

- Need the ability in MessagePack to identify the buffer size, without packing the data. Alternatively to speed up development we could pack the data into a temporary "recycled" memory area and just extract the size.
- The lua workflow is different. MessagePack needs to parse the input lua table to extract the size. Then the lua layer needs to ask NML's C module for a buffer of the required size, and forward that buffer to MessagePack's packer api. 
- MessagePack outputs userdata, its output is now opaque to lua.

###2.5 Embed MessagePack into NML through NML's lua layer, and provide nanomsg's C low level memory management callbacks to MessagePack.

(same drawing as #4)

    ------------------------------------------------------------------
    |  application      |                        NML                  |
    |                   |---------------------------------|           |
    |                   |         MessagePack             |           |     
    {my_big_lua_table} --> MSGPACK:[{my_big_lua_table}] <--> nanomsg --> socket 
    |                (lua types)                      (lua userdata)  |
    -------------------------------------------------------------------

This would require NML's lua layer to fetch the memory allocation callbacks from NML's C module. Then pass these callbacks to MessagePack. Both modules would need to remain in memory as long as they are referencing each other.

**+**

- The most efficient approach.

**-**

- Need to modify CMsgPack to support custom allocators.
- Need the ability to pass C style callbacks between two lua C modules, through the lua api. 
- Need to find an elegant way to implement this and not hack it through a backdoor.

--- 

#3. memory management callbacks implementation details
###3.1 nml
    nml.allocmsg, nml.freemsg, nml.reallocmsg

They're the equivalent to C's malloc, free, realloc, and will be added to the NML binding. No need to get into the details these are simple calls.

###3.2 MessagePack
We need to modify the C module to accept the above callbacks in its pack and unpack methods.
The callbacks will always go through the lua layer before executing their C code, which will incur some overhead:

    MessagePack --> Lua --> NML
              (pcall)  

Add a global singleton structure with the Lua state, and the memory management functions.

    static struct MessagePack{
        lua_State* L;
        fn_malloc malloc;
        fn_free free;
        fn_realloc realloc;
    }g_MessagePack;

Add a global lua_State* accessible to the allocators. When pack/unpack is called, set the global pointers to point to malloc, realloc, free. Pack example:

     static int mp_pack(lua_State* L)
     {
         // not a huge fan of this!
         g_MessagePack.L = L;

         // look for the allocators
         if L(-1) is a table and the table contains the malloc, free, and realloc functions then
             set the function pointers in g_MessagePack

         ...

         // we can't keep this context around
         g_MessagePack.L = NULL;

         // reset the malloc, realloc, free to point to the C default implementation
     }

Add a lua-based malloc, realloc and free functions. These need to pcall the specified lua function. Alloc example:
    void* l_alloc(size_t size)
    {
       void* pv;
       size_t len;

       // this will execute code located in NML's C module
       lua_pushinteger(size);
       lua_call(g_L, 1, LUA_MULTRET);

       pv = lua_tolstring(g_L, -1, &len);
       assert(len==size);
       return pv;
    }

Modify the MessagePack code to use the new memory allocation functions in g_MessagePack. Replace all malloc, free, realloc.

###3.3 messagepack.pack(..., {malloc=fn1, realloc=fn2, free=fn3})
if the stack's top parameter is a table, and the table contains a malloc function and a realloc function and a free function then
- set the malloc, realloc, free global pointers to point to the specified callbacks.

---

#4. Binary serialization
We need the ability to serialize lua (light)userdata (ud).
The problem with ud is that we don't know its size, there's no standard mechanism to determine the size of the data pointed to by a ud payload.

We'll implement our own custom payload package. This package will specify a buffer (ud), the buffer type as a string and its size: 

    { buffer = <some_userdata_value>, buffer_ud_type = "dsx_filter_buffer", buffer_ud_size = <the_size> }

This binary serialization representation will be universal across all MC modules.

###4.1 MessagePack binary serialization
In messagepack.pack:
- if the type is a table then parse the table for the "buffer=", "buffer_ud_type=", "buffer_ud_size="" fields. 
-- if found then encode the specified buffer as a binary format. Preserve the table.
TODO: continue here...
---

#5. protocol disambiguation
Protocol selection is dynamic (specified on every send, and returned on every receive). We'll use nanomsg's control data to send the protocol type.

The protocol type will be specified as a string. Protocol disambiguation should be flexible, and not expect an exact, case sensitive string. This is handled at the lua level.

For example the following protocol type strings should all be resolved to "MessagePack":

- lua-cmessagepack
- messagepack
- lua-messagepack
- msgpack
- MessagePack
etc...

---

#6. API
###6.1 nml.allocmsg

    local pv = nml.allocmsg(size)

Allocates a nanomsg buffer to be used in zero-copy SP socket transfers.

#####parameters

size (number): the desired size in bytes.

#####return values

success:

- [1]: ((light)userdata) the pointer to the allocated nanomsg memory buffer

error:

- [1]: nil
- [2]: (string) a helpful description of the error

#####remarks

This is essentially a malloc of a custom nanomsg-defined structure, with the pointer being offset to obfuscate control data.

---

###6.2 nml.freemsg

    nml.freemsg(pv)

Frees a memory buffer previously allocated by nml.allocmsg.

#####parameters

pv ((light)userdata): a memory buffer allocated by nml.allocmsg

#####return values

success:

- [1]: (number) 0

error:

- [1]: nil
- [2]: (string): a helpful description of the error

#####remarks

The supplied pv will be offset to point to the control data. If the buffer passed in parameter is not a nanomsg buffer this method's behaviour is unknown.

---

###6.3 nml.send

    nml.send(socket, buffer, flags, size, protocol)

Sends a buffer to an SP socket, using the specified protocol.

#####parameters

- socket (number): the SP socket
- buffer ((light)userdata or string): the buffer to send
- flags (number): set to NN_DONTWAIT if non-blocking, or 0.
- size (optional, number): the size in bytes, or the NN_MSG constant, or nil.
- protocol (optional, string): the protocol/encoder used to pack the buffer.

#####return values
    
success:

- [1]: (number) 0

error: 

- [1]: nil
- [2]: (string) a helpful description of the error

#####remarks
Set size to NN_MSG to send using the zero-copy mechanism.

#####implementation
If the buffer type is a string then
- get lua string size
- nn_allocmsg(lua_str_size)
- copy the string into the buffer
- nn_allocmsg the controldata (sizeof("string"))
- copy "string" into control data

If the buffer type is a userdata and the size is NN_MSG and the protocol is not nil then
- fill the msghdr with the buffer and size (NN_MSG)
- nn_allocmsg the control data (sizeof(protocol)) // the size of the string used to specify the protocol
- copy the protocol string into the control data

Else If the buffer type is a userdata and the size is > 0 and protocol is not nil then
- nn_allocmsg(size)
- copy the data into the buffer
- nn_allocmsg(sizeof(protocol))
- copy the protocol string into control data

All success cases:
- nn_sendmsg(msghdr, flags)

---

###6.4 nml.recv

     local buf, received = nml.recv(socket, flags, ud, size)

Receives a message coming from an SP socket and pushes it to lua as a string buffer.

#####parameters

- socket (number): the SP socket.
- flags (number): set to NN_DONTWAIT if non-blocking, or 0.
- ud ((light)userdata): points to a (light)userdata, or nil. 
- size (number): size in bytes, or NN_MSG, or nil.

#####return values

success:

- [1]: ((light)userdata or string)the buffer
- [2]: (number) the number of bytes written to the buffer

error:

- [1]: nil
- [2]: (string) a helpful description of the error

---

#7. work breakdown


1. Write binary serialization tests for cmsgpack.
2. Implement binary serialization in cmsgpack.
--> M1: binary serialization tested successfully

3. Write MessagePack/NML memory allocation tests, using Pack/Unpack and passing malloc/realloc/free.
4. Implement NML memory allocation binding. (needed for next)
5. Implement MessagePack memory allocators.
--> M2: allocators tested successfully
--> M3: cmsgpack code complete

4. Write NML dynamic protocol selection tests, test JSON, MessagePack, strings.
5. Implement NML dynamic protocol selection
--> M2 dynamic protocols tested successfully.
--> RC1 all modules code complete