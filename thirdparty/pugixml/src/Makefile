CC=g++-4.8
CFLAGS=-std=c++11
CDEBUG=$(CFLAGS) -g0
CRELEASE=$(CFLAGS) -g3

SOURCES=*.cpp
HEADERS=*.hpp

all: debug release

debug: $(SOURCES) $(HEADERS)
	$(CC) $(CDEBUG) -c $(SOURCES)
	ar -cvq ../lib/libpugixmld.a *.o
	rm -f *.o
	
release: $(SOURCES) $(HEADERS)
	$(CC) $(CRELEASE) -c $(SOURCES)
	ar -cvq ../lib/libpugixml.a *.o
	rm -f *.o
	
clean:
	rm -f *.o
	rm -f ../lib/*.a
