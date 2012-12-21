export CC=gcc
export CFLAGS=-W -Wall

export UNAME=$(shell uname)

# Detect OS type
ifeq (Linux,$(UNAME))
export EXEC=NoSQLDB
export LDFLAGS=-lpthread
else ifeq (CYGWIN_NT-5.1,$(UNAME))
export EXEC=NoSQLDB.exe
export LDFLAGS=-lpthread
else ifeq (MINGW32_NT-5.1,$(UNAME))
export EXEC=NoSQLDB.exe
export LDFLAGS=-lws2_32
else
export EXEC=NoSQLDB
export LDFLAGS=-lpthread
endif

.PHONY: $(EXEC) clean mrproper

all: $(EXEC)

$(EXEC):
	@(cd src && $(MAKE))
	cp src/$(EXEC) .

clean:
	@(cd src && $(MAKE) $@)
	
mrproper:
	@(cd src && $(MAKE) $@)
	rm -rf $(EXEC)