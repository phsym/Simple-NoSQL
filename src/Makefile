#MKDEP=gcc -MM -o .depend *.c
#MKDEP=gcc -MM *.c > .depend
SRC=$(wildcard *.c)
HDR=$(wildcard *.h)
OBJ=$(SRC:.c=.o)

OUT_DIR=./bin/

OBJS=$(addprefix $(OUT_DIR), $(OBJ))

.PHONY: clean mrproper

all: $(EXEC)

# depend: .depend

# .depend: $(SRC) $(HDR)
	# $(MKDEP)
	
# -include .depend
	
$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OUT_DIR)%.o: %.c $(OUT_DIR)
	$(CC) -o $@ -c $< $(CFLAGS)
	
$(OUT_DIR):
	mkdir $(OUT_DIR)

clean:
	rm -rf bin
	
mrproper: clean
	rm -rf $(EXEC)