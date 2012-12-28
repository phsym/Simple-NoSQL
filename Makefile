DEBUG=yes
export CC=gcc
export CFLAGS=-W -Wall

ifeq ($(DEBUG), yes)
	CFLAGS:=$(CFLAGS) -g
else
	CFLAGS:=$(CFLAGS) -O2
endif

export UNAME=$(shell uname)

# Detect OS type
ifneq (, $(findstring Linux, $(UNAME)))
export EXEC=NoSQLDB
export LDFLAGS=-lpthread
else ifneq (, $(findstring CYGWIN, $(UNAME)))
export EXEC=NoSQLDB.exe
export LDFLAGS=-lpthread
else ifneq (, $(findstring MINGW32, $(UNAME)))
export EXEC=NoSQLDB.exe
export LDFLAGS=-lws2_32
endif

.PHONY: $(EXEC) clean mrproper

all: $(EXEC)

$(EXEC):
	@(cd src && $(MAKE))
	cp src/bin/$(EXEC) .

clean:
	@(cd src && $(MAKE) $@)
	
mrproper:
	@(cd src && $(MAKE) $@)
	rm -rf $(EXEC)