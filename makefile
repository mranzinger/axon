CC=g++
FLAGS=-std=c++11 -g -msse4.2 -fPIC
DFLAGS=$(FLAGS)
RFLAGS=-O3 $(FLAGS)

THIRD_PARTY ?= /home/mike/dev/ThirdParty
SNAPPY_PATH ?= $(THIRD_PARTY)/snappy/install
LIBEVENT_PATH ?= $(THIRD_PARTY)/libevent/install

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
SER_DEMO_SRC = $(SRC_DEMO)/serialization_demo.cpp

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

LIBS = lib/libaxutil.a lib/libaxutil.so \
       lib/libaxser.a lib/libaxser.so \
       lib/libaxcomm.a lib/libaxcomm.so

LIBS_D = lib/libaxutild.a lib/libaxutild.so \
         lib/libaxserd.a lib/libaxserd.so \
         lib/libaxcommd.a lib/libaxcommd.so

EXES_D = demo/client_demo_debug demo/server_demo_debug demo/serialization_demo_debug
EXES_R = demo/client_demo_release demo/server_demo_release demo/serialization_demo_release
EXES = $(EXES_D) $(EXES_R)

INCLUDES= -Iinclude \
		  -Iinclude/serialization \
		  -Iinclude/communication \
		  -Ithirdparty/rapidjson/include \
		  -Ithirdparty/pugixml/src \
          -I$(SNAPPY_PATH)/include \
          -I$(LIBEVENT_PATH)/include

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
	$(CC) $(DFLAGS) -c $< $(INCLUDES) -o $@ 
	
$(OBJ_UTIL)/%.o: $(SRC_UTIL)/%.cpp
	$(CC) $(RFLAGS) -c $< $(INCLUDES) -o $@ 
	
$(OBJ_SER)/%.od: $(SRC_SER)/%.cpp
	$(CC) $(DFLAGS) -c $< -o $@ $(INCLUDES) \
			-Lthirdparty/pugixml/lib \
			-Llib \
			-laxutild \
			-lpugixmld \
            -L$(SNAPPY_PATH)/lib -lsnappy

$(OBJ_SER)/%.o: $(SRC_SER)/%.cpp
	$(CC) $(RFLAGS) -c $< -o $@ $(INCLUDES) \
			-Lthirdparty/pugixml/lib \
			-Llib \
			-laxutil \
			-lpugixml \
            -L$(SNAPPY_PATH)/lib -lsnappy

$(OBJ_COMM)/%.od: $(SRC_COMM)/%.cpp
	$(CC) $(DFLAGS) -c $< -o $@ $(INCLUDES) \
			-Lthirdparty/pugixml/lib \
			-Llib \
			-laxserd -laxutild \
			-lpugixmld \
			-L$(LIBEVENT_PATH)/lib -levent -levent_pthreads -levent_core \
            -L$(SNAPPY_PATH)/lib -lsnappy

$(OBJ_COMM)/%.o: $(SRC_COMM)/%.cpp
	$(CC) $(RFLAGS) -c $< -o $@ $(INCLUDES) \
			-Lthirdparty/pugixml/lib \
			-Llib \
			-laxser -laxutil \
			-lpugixml \
			-L$(LIBEVENT_PATH)/lib -levent -levent_pthreads -levent_core \
            -L$(SNAPPY_PATH)/lib -lsnappy

lib/libaxutild.a: $(UTIL_OBJS_D)
	ar rvs $@ $^

lib/libaxutild.so: $(UTIL_OBJS_D)
	$(CC) $(DFLAGS) -shared -o $@ $^ -lrt $(INCLUDES)
	
lib/libaxserd.a: $(SER_OBJS_D)
	ar rvs $@ $^

lib/libaxserd.so: $(SER_OBJS_D) lib/libaxutild.so
	$(CC) $(DFLAGS) -shared -o $@ $(SER_OBJS_D) $(INCLUDES) \
                -Llib -Lthirdparty/pugixml/lib \
                -laxutild \
                -lpugixmld \
                -L$(SNAPPY_PATH)/lib -lsnappy

lib/libaxcommd.a: $(COM_OBJS_D)
	ar rvs $@ $^

lib/libaxcommd.so: $(COM_OBJS_D) lib/libaxserd.so
	$(CC) $(DFLAGS) -shared -o $@ $(COM_OBJS_D) $(INCLUDES) \
            -Llib \
            -laxutild -laxserd \
            -L$(LIBEVENT_PATH)/lib -levent -levent_pthreads \
            -L$(SNAPPY_PATH)/lib -lsnappy

lib/libaxutil.a: $(UTIL_OBJS)
	ar rvs $@ $^

lib/libaxutil.so: $(UTIL_OBJS)
	$(CC) $(RFLAGS) -shared -o $@ $^ -lrt $(INCLUDES)

lib/libaxser.a: $(SER_OBJS)
	ar rvs $@ $^
	
lib/libaxser.so: $(SER_OBJS) lib/libaxutil.so
	$(CC) $(RFLAGS) -shared -o $@ $(SER_OBJS) $(INCLUDES) \
            -Llib -Lthirdparty/pugixml/lib \
            -laxutil \
            -lpugixml \
            -L$(SNAPPY_PATH)/lib -lsnappy

lib/libaxcomm.a: $(COM_OBJS)
	ar rvs $@ $^

lib/libaxcomm.so: $(COM_OBJS) lib/libaxser.so
	$(CC) $(RFLAGS) -shared -o $@ $(COM_OBJS) $(INCLUDES) \
            -Llib \
            -laxutil -laxser \
            -L$(LIBEVENT_PATH)/lib -levent -levent_pthreads

demo/serialization_demo_debug: $(SER_DEMO_SRC) $(LIBS_D)
	$(CC) $(DFLAGS) $(SER_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxserd -laxutild -lpugixmld \
		-L$(SNAPPY_PATH)/lib -lsnappy

demo/serialization_demo_release: $(SER_DEMO_SRC) $(LIBS)
	$(CC) $(RFLAGS) $(SER_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxser -laxutil -lpugixml \
		-L$(SNAPPY_PATH)/lib -lsnappy

demo/client_demo_debug: $(CLIENT_DEMO_SRC) $(LIBS_D)
	$(CC) $(DFLAGS) $(CLIENT_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxcommd -laxserd -laxutild -lpugixmld \
		-L$(LIBEVENT_PATH)/lib -levent -levent_pthreads \
        -L$(SNAPPY_PATH)/lib -lsnappy
        
demo/client_demo_release: $(CLIENT_DEMO_SRC) $(LIBS)
	$(CC) $(RFLAGS) $(CLIENT_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxcomm -laxser -laxutil -lpugixml \
		-L$(LIBEVENT_PATH)/lib -levent -levent_pthreads \
        -L$(SNAPPY_PATH)/lib -lsnappy

demo/server_demo_debug: $(SERVER_DEMO_SRC) $(LIBS_D)
	$(CC) $(DFLAGS) $(SERVER_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxcommd -laxserd -laxutild -lpugixmld \
		-L$(LIBEVENT_PATH)/lib -levent -levent_pthreads \
        -L$(SNAPPY_PATH)/lib -lsnappy
        
demo/server_demo_release: $(SERVER_DEMO_SRC) $(LIBS)
	$(CC) $(RFLAGS) $(SERVER_DEMO_SRC) -o $@ \
		-Iinclude \
		-Llib \
		-Lthirdparty/pugixml/lib \
		-laxcomm -laxser -laxutil -lpugixml \
		-L$(LIBEVENT_PATH)/lib -levent -levent_pthreads \
        -L$(SNAPPY_PATH)/lib -lsnappy
        
clean:
	rm -rf lib
	rm -rf obj
	rm -rf $(EXES)
