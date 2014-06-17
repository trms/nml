#NML MessagePack
This document outlines the details of NML's MessagePack support.

#Goal
Provide an efficient, binary-compatible, seamless and extensible encoding and decoding mechanism as part of NML.

#Current problem
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

###Notes on MessagePack

There are some important aspects of MessagePack that we should be aware of.

- I am currently basing my MessagePack assumptions on this binding: https://github.com/antirez/lua-cmsgpack
- that MessagePack binding uses a BSD two clause license, I will probably need to update this project's license (MIT)
- **MessagePack is Big Endian**, this may cause a performance issue when transferring larger data buffers across little endian systems. All of our MC components are little endian at the moment.
- MessagePack encoding and decoding are two step processes. They both allocate a temporary internal buffer to store the encoded data, which is then pushed to lua as a string buffer.

#solutions

I propose that MessagePack remains an independent C module with a lua binding. It will be made into a NuGet that can be imported by NML and any other modules requiring lua C MessagePack support. This means that buffers still need to be exposed in lua. Possible ways to handle this situation are detailed below; 

Note: I'm going to refer to the layer on top of NML as "the application" for simplicity.

###1. Use MessagePack at the application level, by swapping out the JSON encoding. 
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

###2. Embed MessagePack into NML through NML's Lua layer, without modifying MessagePack.

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

###3. Embed MessagePack into NML through NML's lua layer, and modify MessagePack to output a recycled userdata buffer.

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

###4. Embed MessagePack into NML through NML's lua layer, and leverage NML's buffer management api.

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

###5. Embed MessagePack into NML through NML's lua layer, and provide nanomsg's C low level memory management callbacks to MessagePack.

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

#Implementation details
The implementation effort is split between 3 areas: NML's C module, NML's lua layer and MessagePack.

##NML's C module implementation details
Here we need suport 

###nml.allocmsg

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

###nml.freemsg

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

###nml.send

    nml.send(socket, buffer, flags, size)

Sends a buffer to an SP socket.

#####parameters

- socket (number): the SP socket
- buffer ((light)userdata or string): the buffer to send
- flags (number): set to NN_DONTWAIT if non-blocking, or 0.
- size (optional, number): the size in bytes, or the NN_MSG constant, or nil.

#####return values

success:

- [1]: (number) 0

error: 

- [1]: nil
- [2]: (string) a helpful description of the error

#####remarks
Set size to NN_MSG to send using the zero-copy mechanism.

---

###nml.recv

     local pv, received = nml.recv(socket, flags, ud, size)

Receives a message coming from an SP socket.

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

#####remarks
If size is NN_MSG then pv will be set to a (light)userdata. This buffer can then be passed to the protocol decoder for processing. In this case the user needs to call nml.freemsg(pv) in order for the buffer to be properly released.

If size is set to a value > 0, then the payload will be copied into the ud parameter. If the specified size is too small the payload will be truncated.

If the size is nil (not specified), then pv will return the buffer inside a lua string.

---

###TODO: protocol negociation
Both ends need to know what to expect. There is no mixing of protocols (regular strings vs MessagePack), otherwise I need to disambiguate them.

---

#TODO: estimation of the work