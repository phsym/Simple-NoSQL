# MKDEP=gcc -MM -o .depend *.c
SRC=$(wildcard ./*.c)
HDR=$(wildcard *.h)
OBJ=$(SRC:.c=.o)

all: $(EXEC)

# depend: .depend

# .depend: $(SRC) $(HDR)
	# $(MKDEP)
	
# -include .depend
	
$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o
	
mrproper: clean
	rm -rf $(EXEC)
	rm -rf $(EXEC).exe