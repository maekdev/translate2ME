# Makefile
#
# 240916
# markus ekler
#
# experimental Makefile to compile most recent libtre (mingw32 / windows host)
# we need the following parameters CC, AR, RM

CC = @tcc
AR = @tcc -ar
RM = @del

SRC=tre-ast.c \
	tre-compile.c \
	tre-match-backtrack.c \
	tre-match-parallel.c \
	tre-mem.c \
	tre-parse.c \
	tre-stack.c \
	regcomp.c \
	regexec.c \
	regerror.c \
	tre-match-approx.c
	
	
SRC_DIR=.\tre\lib
OBJ = $(SRC:%.c=$(SRC_DIR)\\%.o)
CFLAGS=-I$(SRC_DIR)
CFLAGS+=-I.\tre\win32
CFLAGS+=-DHAVE_CONFIG_H

all: libtre.a

rebuild: clean all

libtre.a: $(OBJ)
	@echo "=== building library ($@) ==="
	$(AR) cru libtre.a $(OBJ)
	
distclean: clean
	@echo "=== remove library ==="
	$(RM) libtre.a

clean:
	@echo "=== clean up ==="
	$(RM) $(SRC_DIR)\*.o

$(SRC_DIR)\\%.o : $(SRC_DIR)\\%.c
	@echo "=== compile ($<) ==="
	$(CC) $(CFLAGS) -c $< -o $@