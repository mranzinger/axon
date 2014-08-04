CC=g++-4.7
CFLAGS=-std=c++11 -fPIC
CDEBUG=$(CFLAGS) -g
CRELEASE=$(CFLAGS) -O3

SOURCES=*.cpp
HEADERS=*.hpp

all: debug release

debug: $(SOURCES) $(HEADERS)
	$(CC) $(CDEBUG) -c $(SOURCES)
	$(CC) $(CDEBUG) -shared -o ../lib/libpugixmld.so *.o
	ar -cvq ../lib/libpugixmld.a *.o
	rm -f *.o
	
release: $(SOURCES) $(HEADERS)
	$(CC) $(CRELEASE) -c $(SOURCES)
	$(CC) $(CRELEASE) -shared -o ../lib/libpugixml.so *.o 
	ar -cvq ../lib/libpugixml.a *.o
	rm -f *.o
	
clean:
	rm -f *.o
	rm -f ../lib/*.a ../lib/*.so
