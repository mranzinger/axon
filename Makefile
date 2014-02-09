CC=g++
FLAGS=-std=gnu++11 -g
DFLAGS=$(FLAGS)
RFLAGS=-O3 $(FLAGS)

SRC_ROOT = src

SRC_UTIL = $(SRC_ROOT)/util
SRC_SER = $(SRC_ROOT)/serialization
SRC_COMM = $(SRC_ROOT)/communication
SRC_DEMO = demo/src

OBJ_ROOT = obj

OBJ_UTIL = $(OBJ_ROOT)/util
OBJ_SER = $(OBJ_ROOT)/serialization
OBJ_COMM = $(OBJ_ROOT)/communication

UTIL_SRC = $(wildcard $(SRC_UTIL)/*.cpp)
SER_SRC = $(wildcard $(SRC_SER)/*.cpp)
COM_SRC = $(wildcard $(SRC_COMM)/*.cpp)
DEMO_SRC = $(wildcard $(SRC_DEMO)/*.cpp)

UTIL_OBJS = $(patsubst $(SRC_ROOT)/util/%.cpp,$(OBJ_UTIL)/%.o,$(UTIL_SRC))
SER_OBJS = $(patsubst $(SRC_ROOT)/serialization/%.cpp,$(OBJ_ROOT)/serialization/%.o,$(SER_SRC))
COM_OBJS = $(patsubst $(SRC_ROOT)/communication/%.cpp,$(OBJ_ROOT)/communication/%.o,$(COM_SRC))
DEMO_OBJS =

UTIL_OBJS_D = $(patsubst %.o,%.od,$(UTIL_OBJS))
SER_OBJS_D = $(patsubst %.o,%.od,$(SER_OBJS))
COM_OBJS_D = $(patsubst %.o,%.od,$(COM_OBJS))
DEMO_OBJS_D = 

SRC = $(UTIL_SRC) $(SER_SRC) $(COM_SRC)
OBJS = $(UTIL_OBJS) $(SER_OBJS) $(COM_OBJS)
OBJS_D = $(UTIL_OBJS_D) $(SER_OBJS_D) $(COM_OBJS_D)

LIBS = lib/libaxutil.a lib/libaxser.a
LIBS_D = lib/libaxutild.a lib/libaxserd.a

EXES = demo/demo_debug demo/demo_release

.PHONY: all clean setup

all: debug release

debug: setup $(OBJS_D) $(LIBS_D) demo/demo_debug

release: setup $(OBJS) $(LIBS) demo/demo_release

setup:
	mkdir -p lib
	mkdir -p $(OBJ_UTIL)
	mkdir -p $(OBJ_SER)
	mkdir -p $(OBJ_COMM)

$(OBJ_UTIL)/%.od: $(SRC_UTIL)/%.cpp
	$(CC) $(DFLAGS) -c $< -Iinclude -Iinclude/util -o $@ 
	
$(OBJ_UTIL)/%.o: $(SRC_UTIL)/%.cpp
	$(CC) $(RFLAGS) -c $< -Iinclude -Iinclude/util -o $@ 
	
$(OBJ_SER)/%.od: $(SRC_SER)/%.cpp
	$(CC) $(DFLAGS) -c $< -o $@ \
			-Iinclude \
			-Iinclude/serialization \
			-Ithirdparty/include \
			-Lthirdparty/pugixml/lib \
			-lpugixmld

$(OBJ_SER)/%.o: $(SRC_SER)/%.cpp
	$(CC) $(RFLAGS) -c $< -o $@ \
			-Iinclude \
			-Iinclude/serialization \
			-Ithirdparty/include \
			-Lthirdparty/pugixml/lib \
			-lpugixml

lib/libaxutild.a: $(UTIL_OBJS_D)
	ar rvs $@ $^
	
lib/libaxserd.a: $(SER_OBJS_D)
	ar rvs $@ $^

lib/libaxutil.a: $(UTIL_OBJS)
	ar rvs $@ $^
	
lib/libaxser.a: $(SER_OBJS)
	ar rvs $@ $^

demo/demo_debug: $(DEMO_SRC) $(LIBS_D)
	$(CC) $(DFLAGS) $(DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxserd -laxutild -lpugixmld

demo/demo_release: $(DEMO_SRC) $(LIBS)
	$(CC) $(RFLAGS) $(DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxser -laxutil -lpugixml

clean:
	rm -rf lib
	rm -rf obj
