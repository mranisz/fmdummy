CXX=g++
CFLAGS=-Wall -std=c++11 -O3 -mpopcnt
	
all: testFMDummy

testFMDummy: libfmdummy.a libs/libaelf64.a
	$(CXX) $(CFLAGS) testFMDummy.cpp libfmdummy.a libs/libaelf64.a -o testFMDummy

libfmdummy.a: fmdummy.h fmdummy.cpp shared/common.h shared/common.cpp shared/patterns.h shared/patterns.cpp shared/sais.h shared/sais.c shared/timer.h shared/timer.cpp shared/xxhash.h shared/xxhash.cpp shared/hash.h shared/hash.cpp shared/wt.h shared/wt.cpp shared/api.h
	$(CXX) $(CFLAGS) -c fmdummy.cpp shared/common.cpp shared/patterns.cpp shared/sais.c shared/timer.cpp shared/xxhash.cpp shared/hash.cpp shared/wt.cpp
	ar rcs libfmdummy.a fmdummy.o common.o patterns.o sais.o timer.o xxhash.o hash.o wt.o
	make cleanObjects

cleanObjects:
	rm -f *o

clean:
	rm -f *o testFMDummy libfmdummy.a