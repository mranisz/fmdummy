CXX=g++
CFLAGS=-Wall -std=c++11 -O3
	
all: testFMDummy

testFMDummy: libfmdummy.a libs/libaelf64.a
	$(CXX) $(CFLAGS) testFMDummy.cpp libfmdummy.a libs/libaelf64.a -o testFMDummy

libfmdummy.a: fmdummy.cpp shared/common.cpp shared/patterns.cpp shared/sais.c shared/timer.cpp
	$(CXX) $(CFLAGS) -c fmdummy.cpp shared/common.cpp shared/patterns.cpp shared/sais.c shared/timer.cpp
	ar rcs libfmdummy.a fmdummy.o common.o patterns.o sais.o timer.o
	make cleanObjects

cleanObjects:
	rm -f *o

clean:
	rm -f *o testFMDummy libfmdummy.a