LIB = core

LUA = lua
LUAV = 5.2
SRC=./src
DOC = ./doc

CC = gcc -c 
LN = gcc -O 
ARCHBITS64 = -m64
DEBUG = -g



INSTALL_TOP= $(MEDIA_CIRCUS_PREFIX)
INSTALL_BIN= $(INSTALL_TOP)/bin
INSTALL_INC= $(INSTALL_TOP)/include
INSTALL_LIB= $(INSTALL_TOP)/lib
INSTALL_MAN= $(INSTALL_TOP)/doc/$(LUA)/$(LIB)
INSTALL_LMOD= $(INSTALL_LIB)/$(LUA)/$(LUAV)
INSTALL_CMOD= $(INSTALL_LIB)/$(LUA)/$(LUAV)/nml

# What to install.
TO_BIN=
TO_INC=
TO_LIB= $(LIB).so
TO_MAN= 

# How to install. If your install program does not support "-p", then
# you may have to run ranlib on the installed liblua.a.
INSTALL_DATA=  cp -p
#
# If you don't have "install" you can use "cp" instead.
# INSTALL= cp -p
# INSTALL_EXEC= $(INSTALL)
# INSTALL_DATA= $(INSTALL)

MKDIR= mkdir -p
RM= rm -f

all: $(LIB)

$(LIB):
	cd $(SRC) && $(MAKE) $@

install: dummy
	cd $(SRC) && $(MKDIR) $(INSTALL_BIN) $(INSTALL_INC) $(INSTALL_LIB) $(INSTALL_MAN) $(INSTALL_LMOD) $(INSTALL_CMOD)
	cd $(SRC) && $(INSTALL_DATA) $(TO_LIB) $(INSTALL_CMOD)

uninstall:
	$(RM) $(INSTALL_CMOD)/$(TO_LIB)

clean:
	cd $(SRC) && $(RM) *.so *.o 

# make may get confused with test/ and install/
dummy:
