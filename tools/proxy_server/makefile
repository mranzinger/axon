BOOST_PATH		?= $(THIRD_PARTY)/boost_1_55_0
AXON_PATH		?= ../..
LIBEVENT_PATH   ?= $(THIRD_PARTY)/libevent/install

SYMS	 :=
#SYMS	 += -g
OPTLEVEL :=
#OPTLEVEL += -O3

CC := g++-4.7 -std=c++11 $(SYMS) $(OPTLEVEL)

INCLUDES := -I$(BOOST_PATH)/include \
			-I$(AXON_PATH)/include \
			-I$(LIBEVENT_PATH)/include
			
LDFLAGS := -L$(BOOST_PATH)/lib \
		   -L$(AXON_PATH)/lib \
		   -L$(LIBEVENT_PATH)/lib

LIB := -laxutild -laxserd -laxcommd \
	   -levent -levent_pthreads \
       -lboost_program_options
       
TOOL_SRC := proxy_server.cpp

TOOL_EXE := $(patsubst %.cpp,%,$(TOOL_SRC))

CXXFILES := $(wildcard *.cpp)
CXXDEPS := $(wildcard *.h)
CXXOBJS := $(patsubst %.cpp,obj/%.o,$(CXXFILES))

all: setup $(TOOL_EXE)

setup:
	mkdir -p obj
	
$(TOOL_EXE) : $(CXXOBJS)
	$(CC) -o $(TOOL_EXE) $(CXXOBJS) $(LDFLAGS) $(LIB)
	
obj/%.o : %.cpp $(CXXDEPS)
	$(CC) -o $@ -c $< $(INCLUDES)
	
.PHONY: clean
clean:
	rm -rf $(TOOL_EXE) obj
	@echo done.