export CC=gcc
export CFLAGS=-W -Wall
export LDFLAGS=-lws2_32
export EXEC=NoSQLDB

all: $(EXEC)

$(EXEC):
	@(cd src && $(MAKE))

clean:
	@(cd src && $(MAKE) $@)
	
mrproper:
	@(cd src && $(MAKE) $@)