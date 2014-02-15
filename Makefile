CC=g++
FLAGS=-std=gnu++11 -g
DFLAGS=$(FLAGS)
RFLAGS=-O3 $(FLAGS)

INC_ROOT = include
SRC_ROOT = src

INC_ALL = $(wildcard $(INC_ROOT)/*.h)

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
COM_SRC = $(wildcard $(SRC_COMM)/*.cpp) $(wildcard $(SRC_COMM)/detail/*.h)

CLIENT_DEMO_SRC = $(SRC_DEMO)/client_demo.cpp
SERVER_DEMO_SRC = $(SRC_DEMO)/server_demo.cpp

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

LIBS = lib/libaxutil.a lib/libaxser.a lib/libaxcomm.a
LIBS_D = lib/libaxutild.a lib/libaxserd.a lib/libaxcommd.a

EXES_D = demo/client_demo_debug demo/server_demo_debug
EXES_R = demo/client_demo_release demo/server_demo_release
EXES = $(EXES_D) $(EXES_R)
	    

.PHONY: all clean setup

all: debug release

debug: setup $(OBJS_D) $(LIBS_D) $(EXES_D) $(INC_ALL)

release: setup $(OBJS) $(LIBS) $(EXES_R) $(INC_ALL)

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
			-Llib \
			-laxutild \
			-lpugixmld

$(OBJ_SER)/%.o: $(SRC_SER)/%.cpp
	$(CC) $(RFLAGS) -c $< -o $@ \
			-Iinclude \
			-Iinclude/serialization \
			-Ithirdparty/include \
			-Lthirdparty/pugixml/lib \
			-Llib \
			-laxutil \
			-lpugixml

$(OBJ_COMM)/%.od: $(SRC_COMM)/%.cpp
	$(CC) $(DFLAGS) -c $< -o $@ \
			-Iinclude \
			-Iinclude/communication \
			-Ithirdparty/include \
			-Lthirdparty/pugixml/lib \
			-Llib \
			-laxserd -laxutild \
			-lpugixmld \
			-levent -levent_pthreads -levent_core

$(OBJ_COMM)/%.o: $(SRC_COMM)/%.cpp
	$(CC) $(RFLAGS) -c $< -o $@ \
			-Iinclude \
			-Iinclude/communication \
			-Ithirdparty/include \
			-Lthirdparty/pugixml/lib \
			-Llib \
			-laxser -laxutil \
			-lpugixml \
			-levent -levent_pthreads -levent_core

lib/libaxutild.a: $(UTIL_OBJS_D)
	ar rvs $@ $^
	
lib/libaxserd.a: $(SER_OBJS_D)
	ar rvs $@ $^
	
lib/libaxcommd.a: $(COM_OBJS_D)
	ar rvs $@ $^

lib/libaxutil.a: $(UTIL_OBJS)
	ar rvs $@ $^
	
lib/libaxser.a: $(SER_OBJS)
	ar rvs $@ $^
	
lib/libaxcomm.a: $(COM_OBJS)
	ar rvs $@ $^

demo/client_demo_debug: $(CLIENT_DEMO_SRC) $(LIBS_D)
	$(CC) $(DFLAGS) $(CLIENT_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxcommd -laxserd -laxutild -lpugixmld \
		-levent

demo/client_demo_release: $(CLIENT_DEMO_SRC) $(LIBS)
	$(CC) $(RFLAGS) $(CLIENT_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxcomm -laxser -laxutil -lpugixml \
		-levent

demo/server_demo_debug: $(SERVER_DEMO_SRC) $(LIBS_D)
	$(CC) $(DFLAGS) $(SERVER_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxcommd -laxserd -laxutild -lpugixmld \
		-levent

demo/server_demo_release: $(SERVER_DEMO_SRC) $(LIBS)
	$(CC) $(RFLAGS) $(SERVER_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxcomm -laxser -laxutil -lpugixml \
		-levent

clean:
	rm -rf lib
	rm -rf obj
