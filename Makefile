# Makefile
#****************************************************************

CC = gcc
CPP = g++
CFLAGS = -Wall -Wno-unused-function -std=c++11
LIBs = -lm
TESTDIR = ./test
INCLUDEDIR = -I./src -I.

PROGRAMS = coarse_hash_table_test \
	fine_hash_table_test \
	atomic_linked_list_test \

all: $(PROGRAMS)

coarse_hash_table_test: $(TESTDIR)/coarse_hash_table_test.cpp
	$(CPP) $(CFLAGS) -o $@ $^ $(INCLUDEDIR) $(LIBS)

fine_hash_table_test: $(TESTDIR)/fine_hash_table_test.cpp
	$(CPP) $(CFLAGS) -o $@ $^ $(INCLUDEDIR) $(LIBS)

atomic_linked_list_test: $(TESTDIR)/atomic_linked_list_test.cpp
	$(CPP) $(CFLAGS) -o $@ $^ $(INCLUDEDIR) $(LIBS)

clean:
	rm -rf $(PROGRAMS) *.o *.a a.out *.err *~