NMSG = nanomsg
LUA = lua
LOCAL=/Users/andrew/src/mediacircus
INCLUDE= -I$(LOCAL)/include
LIBS=$(LOCAL)/libraries
CFLAGS = -L$(LIBS) -I$(INCLUDE) -l$(LUA) -l$(NMSG)

LFLAGS = -O -fpic -L$(LIBS) -l$(NMSG) -I$(INCLUDE) -undefined dynamic_lookup

PIPELINE=pipeline
SURVEY=survey

LUALIB=nml


luananomsg: $(LUALIB).c
	gcc -O2 -c $(INCLUDE) $(LUALIB).c common.c socket.c 
	gcc   $(LFLAGS) $(LUALIB).o common.o socket.o -o $(LUALIB).so

