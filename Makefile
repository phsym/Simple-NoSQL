export CC=gcc
export CFLAGS=-W -Wall
export LDFLAGS=-lws2_32
export UNAME=$(shell uname)

# Detect OS type
ifeq (Linux,$(UNAME))
export EXEC=NoSQLDB
else ifeq (CYGWIN_NT-5.1,$(UNAME))
export EXEC=NoSQLDB.exe
else ifeq (MINGW32_NT-5.1,$(UNAME))
export EXEC=NoSQLDB.exe
else
export EXEC=NoSQLDB
endif

all: $(EXEC)

$(EXEC):
	@(cd src && $(MAKE))
	cp src/$(EXEC) .

clean:
	@(cd src && $(MAKE) $@)
	
mrproper:
	@(cd src && $(MAKE) $@)
	rm -rf $(EXEC)