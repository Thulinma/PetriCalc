SRC = main.cpp petricalc.cpp
OBJ = $(SRC:.cpp=.o)
OUT = PetriCalc
INCLUDES = 
DEBUG = 4
OPTIMIZE = -g
VERSION = `git describe --tags`
CCFLAGS = -Wall -Wextra -funsigned-char $(OPTIMIZE) -DDEBUG=$(DEBUG) -DVERSION=$(VERSION)
CC = $(CROSS)g++
LD = $(CROSS)ld
AR = $(CROSS)ar
LIBS = -ltinyxml 
.SUFFIXES: .cpp 
.PHONY: clean default
default: $(OUT)
.cpp.o:
	$(CC) $(INCLUDES) $(CCFLAGS) $(LIBS) -c $< -o $@
$(OUT): $(OBJ)
	$(CC) $(LIBS) -o $(OUT) $(OBJ)
clean:
	rm -rf $(OBJ) $(OUT) Makefile.bak *~

