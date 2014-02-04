CC=g++-4.8
FLAGS=-std=c++11 
DFLAGS=$(FLAGS)
RFLAGS=-O3 $(FLAGS)

SRC_ROOT = src

SRC_UTIL = $(SRC_ROOT)/util
SRC_SER = $(SRC_ROOT)/serialization
SRC_DEMO = demo/src

OBJ_ROOT = obj

OBJ_UTIL = $(OBJ_ROOT)/util
OBJ_SER = $(OBJ_ROOT)/serialization

UTIL_SRC = $(wildcard $(SRC_UTIL)/*.cpp)
SER_SRC = $(wildcard $(SRC_SER)/*.cpp)
COM_SRC = 
DEMO_SRC = $(wildcard $(SRC_DEMO)/*.cpp)

UTIL_OBJS = $(patsubst $(SRC_ROOT)/util/%.cpp,$(OBJ_UTIL)/%.o,$(UTIL_SRC))
SER_OBJS = $(patsubst $(SRC_ROOT)/serialization/%.cpp,$(OBJ_ROOT)/serialization/%.o,$(SER_SRC))
COM_OBJS = $(patsubst $(SRC_ROOT)/communication/%.cpp,$(OBJ_ROOT)/communication/%.o,$(COM_SRC))
DEMO_OBJS =

SRC = $(UTIL_SRC) $(SER_SRC) $(COM_SRC)
OBJS = $(UTIL_OBJS) $(SER_OBJS) $(COM_OBJS)

LIBS = lib/libaxutild.a lib/libaxserd.a

EXES = demo/demo

.PHONY: all clean setup

all: setup $(OBJS) $(LIBS) $(EXES)

setup:
	mkdir -p lib
	mkdir -p $(OBJ_UTIL)
	mkdir -p $(OBJ_SER)

$(OBJ_UTIL)/%.o: $(SRC_UTIL)/%.cpp
	$(CC) $(DFLAGS) -c $< -Iinclude -Iinclude/util -o $@ 
	
$(OBJ_SER)/%.o: $(SRC_SER)/%.cpp
	$(CC) $(DFLAGS) -c $< -o $@ \
			-Iinclude \
			-Iinclude/serialization \
			-Ithirdparty/include

lib/libaxutild.a: $(UTIL_OBJS)
	ar rvs $@ $^
	
lib/libaxserd.a: $(SER_OBJS)
	ar rvs $@ $^

demo/demo: $(DEMO_SRC) $(LIBS)
	$(CC) $(DFLAGS) $(DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-laxserd -laxutild 

clean:
	rm -rf lib
	rm -rf obj
