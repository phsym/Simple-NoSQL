# MKDEP=gcc -MM -o .depend *.c
CC=gcc
CFLAGS=-W -Wall
LDFLAGS=-lws2_32
EXEC=NoSQLDB
# SRC=$(wildcard ./*.c)
# HDR=$(wildcard *.h)
# OBJ=$(SRC:.c=.o)

SRC=concurrency.c \
config.c \
datastorage.c \
fifo.c \
indextable.c \
linked_list.c \
main.c \
network.c \
protocol.c \
table.c \
utils.c \
worker.c

OBJ=concurrency.o \
config.o \
datastorage.o \
fifo.o \
indextable.o \
linked_list.o \
main.o \
network.o \
protocol.o \
table.o \
utils.o \
worker.o


all: $(EXEC)

.SUFFIXES: .o .c
.c.o:
	$(CC) -o $@ -c $< $(CFLAGS)

# depend: .depend

# .depend: $(SRC) $(HDR)
	# $(MKDEP)
	
# -include .depend
	
NoSQLDB: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

# %.o: %.c %.h
	# $(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o
	
mrproper: clean
	rm -rf $(EXEC)
	rm -rf $(EXEC).exe