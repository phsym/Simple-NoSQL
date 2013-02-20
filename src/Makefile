############################################################
#       MAKEFILE for module compiling source code          #
############################################################


MKDEP=gcc -MM -I $(INCD) *.c | sed -e "s/\(.*\.o\)/bin\/\1/" > Makefile.depend
SRC=$(wildcard *.c)
HDR=$(wildcard *.h)
OBJ=$(SRC:.c=.o)

OUT_DIR=bin/

OBJS=$(addprefix $(OUT_DIR), $(OBJ))
EXECS=$(addprefix $(OUT_DIR), $(EXEC))
MODULES=crypto

.PHONY: clean mrproper $(MODULES)

all: $(EXEC)

$(EXEC): $(EXECS)

depend: Makefile.depend

Makefile.depend: $(SRC) $(HDR)
	@echo "Rebuilding dependencies"
	@($(MKDEP))
	
-include Makefile.depend
	
$(EXECS): $(OUT_DIR) $(MODULES) $(OBJS)
	$(CC) -o $@ $(OBJS) bin/crypto.a $(LDFLAGS)
	
crypto:
	@(cd crypto && $(MAKE))
	cp crypto/bin/crypto.a bin/crypto.a

$(OUT_DIR)%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
	
$(OUT_DIR):
	mkdir -p $(OUT_DIR)
	
clean:
	@(cd crypto && $(MAKE) clean)
	rm -rf bin
	
mrproper: clean