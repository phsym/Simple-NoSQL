###############################################
#       MAKEFILE for module "crypto"          #
###############################################



MKDEP=gcc -MM -I $(INCD) *.c | sed -e "s/\(.*\.o\)/bin\/\1/" > Makefile.depend
SRC=$(wildcard *.c)
HDR=$(wildcard *.h)
OBJ=$(SRC:.c=.o)

OUT_DIR=bin/

OBJS=$(addprefix $(OUT_DIR), $(OBJ))
EXECS=$(addprefix $(OUT_DIR), $(EXEC))

.PHONY: clean mrproper

all: $(EXEC)

$(EXEC): $(EXECS)

depend: Makefile.depend

Makefile.depend: $(SRC) $(HDR)
	@echo "Rebuilding dependencies"
	$(MKDEP)
	
-include Makefile.depend
	
$(EXECS): $(OUT_DIR) $(OBJS)
	ar rcs bin/crypto.a $(OBJS)
	
$(OUT_DIR)%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
	
$(OUT_DIR):
	mkdir -p $(OUT_DIR)
	
clean:
	rm -rf bin
	
mrproper: clean