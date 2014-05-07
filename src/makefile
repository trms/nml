LIB = core

LUA = lua
LUAV = 5.2
SRC=./src


CC = gcc -c 
LN = gcc -O 
ARCHBITS64 = -m64
DEBUG = -g

INSTALL_TOP= $(MEDIA_CIRCUS_PREFIX)
INSTALL_BIN= $(INSTALL_TOP)/bin
INSTALL_INC= $(INSTALL_TOP)/include
INSTALL_LIB= -L$(INSTALL_TOP)/lib -L/usr/local/lib
INSTALL_MAN= $(INSTALL_TOP)/doc/$(LUA)/$(LIB)
INSTALL_LMOD= $(INSTALL_LIB)/$(LUA)/$(LUAV)
INSTALL_CMOD= $(INSTALL_LIB)/$(LUA)/$(LUAV)/nml

CFLAGS = -O2 -Wall -Wno-missing-braces $(ARCHBITS) -I$(INSTALL_INC) 

LFLAGS =  -fpic $(INSTALL_LIB) -lnanomsg -I$(INSTALL_INC) -undefined dynamic_lookup
CORE_O = 
LIB_SRC = nml.c socket.c common.c protocols.c dev_utils.c
LIB_O  = nml.o socket.o common.o protocols.o dev_utils.o
LIB_T  = $(LIB).so


$(LIB): $(LIB_O)
	$(LN) $(LFLAGS) $(LIB_O) -o $(LIB_T)

$(LIB_O): $(LIB_SRC)
	$(CC) $(CFLAGS) $(LIB_SRC)

