# makefile4wortklauber
# 130722
# markus ekler
#
# 240912 (mingw32 compiler on win32 host)


CC   = @x86_64-w64-mingw32-gcc
#CC = @tcc
#WINDRES = @x86_64-w64-mingw32-windres
WINDRES = @windres
#STRIP = @x86_64-w64-mingw32-strip
STRIP = @strip
RM = @del
SRC_DIR = .\src
OBJ_DIR = .\obj
BIN_DIR = .\bin
LIB_DIR = .\lib

BIN  = \
	$(BIN_DIR)\translate2ME.exe 

SRC = \
	$(SRC_DIR)\main.c \
	$(SRC_DIR)\hwnd.c \
	$(SRC_DIR)\hotkey.c \
	$(SRC_DIR)\cfg_file.c \
	$(SRC_DIR)\cfg_parser.c \
    $(SRC_DIR)\search.c \
    $(SRC_DIR)\wortklauber.rc

LIBS =  -ltre -lminiz
INCS =  -I".\src" 
INCS += -I".\lib\tre\local_includes" 
INCS += -I".\lib\tre\lib" 
INCS += -I".\lib\tre\win32"
INCS += -I".\lib\miniz"
CFLAGS = $(INCS) $(CDEFS) -D_DEBUG -D_UNICODE -DUNICODE
OBJ = $(SRC:.c=.o)
OBJ := $(subst .rc,.res,$(OBJ))
OBJ := $(subst $(SRC_DIR),$(OBJ_DIR),$(OBJ))

all: $(LIB_DIR)/libtre.a $(BIN)

rebuild: clean all

clean:
	@echo === cleaning ===
	$(RM) obj\*.o
	$(RM) obj\*.res
	$(RM) $(BIN)

$(OBJ_DIR)\\%.o : $(SRC_DIR)\%.c
	@echo === compile ($<) ===
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)\\%.res : $(SRC_DIR)\%.rc
	@echo === Resource ($<) ===
	$(WINDRES) -i $< --input-format=rc -o $@ -O coff --include-dir $(SRC_DIR)

$(BIN_DIR)\translate2ME.exe : $(OBJ) $(LIB_DIR)\libtre.a
	@echo === link ($@) ===
	$(CC) -o $@ $(OBJ) -L$(LIB_DIR) $(LIBS) -lcomctl32 -lwinmm -lgdi32 -lshell32 -municode -mwindows
	$(STRIP) $@