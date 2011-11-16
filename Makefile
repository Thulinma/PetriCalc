SRC = main.cpp petricalc.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp
OBJ = $(SRC:.cpp=.o)
OUT = PetriCalc
INCLUDES = 
DEBUG = 4
OPTIMIZE = -g
VERSION = `git describe --tags`
CCFLAGS = -Wall -Wextra -funsigned-char $(OPTIMIZE) -DDEBUG=$(DEBUG) -DVERSION=$(VERSION) -DTIXML_USE_STL
MINGPATH=/home/thulinma/cpp/mingw/mingw_cross_env-2.1/usr/i386-mingw32msvc
CC = $(CROSS)g++
LD = $(CROSS)ld
AR = $(CROSS)ar
LIBS =  
.SUFFIXES: .cpp 
.PHONY: clean default
default: $(OUT)
fast:
	make clean default OPTIMIZE=-O2 DEBUG=0
.cpp.o:
	$(CC) $(INCLUDES) $(CCFLAGS) $(LIBS) -c $< -o $@
$(OUT): $(OBJ)
	$(CC) $(LIBS) -o $(OUT) $(OBJ)
clean:
	rm -rf $(OBJ) $(OUT) Makefile.bak *~
windows:
	make clean default OUT=$(OUT).exe OPTIMIZE=-O2 DEBUG=0 CROSS=i386-mingw32msvc-

