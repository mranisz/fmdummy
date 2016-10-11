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

libfmdummy.a: fmdummy.hpp shared/common.hpp shared/patterns.hpp shared/sais.h shared/sais.c shared/timer.hpp shared/xxhash.h shared/xxhash.c shared/hash.hpp shared/huff.hpp
	$(CXX) $(CFLAGS) -c shared/sais.c shared/xxhash.c
	ar rcs libfmdummy.a fmdummy.hpp sais.o xxhash.o shared/common.hpp shared/patterns.hpp shared/timer.hpp shared/hash.hpp shared/huff.hpp
	make cleanObjects

cleanObjects:
	rm -f *o

clean:
	rm -f *o test/countFMDummy libfmdummy.a