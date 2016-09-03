UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
ASMLIB = libaelf64.a
else
ASMLIB = libacof64.lib
endif

CXX=g++
CFLAGS=-Wall -std=c++11 -O3 -mpopcnt
	
all: countFMDummy

countFMDummy: test/countFMDummy.cpp libfmdummy.a libs/$(ASMLIB)
	$(CXX) $(CFLAGS) test/countFMDummy.cpp libfmdummy.a libs/$(ASMLIB) -o test/countFMDummy

libfmdummy.a: fmdummy.hpp shared/common.h shared/common.cpp shared/patterns.h shared/patterns.cpp shared/sais.h shared/sais.c shared/timer.h shared/timer.cpp shared/xxhash.h shared/xxhash.c shared/hash.hpp shared/huff.h shared/huff.cpp
	$(CXX) $(CFLAGS) -c shared/common.cpp shared/patterns.cpp shared/sais.c shared/timer.cpp shared/xxhash.c shared/huff.cpp
	ar rcs libfmdummy.a fmdummy.hpp common.o patterns.o sais.o timer.o xxhash.o huff.o shared/hash.hpp
	make cleanObjects

cleanObjects:
	rm -f *o

clean:
	rm -f *o test/countFMDummy libfmdummy.a