#NML - nano message libray - lua wrapper
This is a thin lua binding of nanomsg. It's pretty much a direct translation of the nanomsg api, with as little modifications as possible. 

The biggest assumption at this time is that a message always gets translated into a lua string. 
On the send side, nml will look for a string in its input parameters, add the null terminator, and send it over to nanomsg.
On the receiving end, nml will expect to get a string from the socket. It will then remove the null terminator and push the string to lua.

#Todo
Support for CMsgPack will be implemented shortly, and probably baked into nml as an optional way to send messages. The plan is to support binary data. It's already all implemented, just needs to be put in here :)