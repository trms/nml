#NML
##nano message libray - lua wrapper
This is a thin lua binding of nanomsg. It's pretty much a direct translation of the nanomsg api, with as little modifications as possible. 

The biggest assumption at this time is that a message always gets translated into a lua string. 
On the send side, nml will look for a string in its input parameters, add the null terminator, and send it over to nanomsg.
On the receiving end, nml will expect to get a string from the socket. It will then remove the null terminator and push the string to lua.

##license
This project is licensed under the MIT license.

Copyright (c) 2014 Tightrope Media Systems Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

##todo
Support for CMsgPack will be implemented shortly, and probably baked into nml as an optional way to send messages. The plan is to support binary data. It's already all implemented, just needs to be put in here :)