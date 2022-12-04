# Makefile
#****************************************************************

CC = gcc
CPP = g++
CFLAGS = -Wall -Wno-unused-function -std=c++17 -pthread -g -march=native #-fsanitize=address
LIBs = -lm
TESTDIR = ./test
INCLUDEDIR = -I./src -I.

PROGRAMS = coarse_hash_table_test \
	fine_hash_table_test \
	atomic_linked_list_test \
	lock_free_hash_table_test \

all: $(PROGRAMS)

coarse_hash_table_test: $(TESTDIR)/coarse_hash_table_test.cpp
	$(CPP) $(CFLAGS) -o $@ $^ $(INCLUDEDIR) $(LIBS)

fine_hash_table_test: $(TESTDIR)/fine_hash_table_test.cpp
	$(CPP) $(CFLAGS) -o $@ $^ $(INCLUDEDIR) $(LIBS)

atomic_linked_list_test: $(TESTDIR)/atomic_linked_list_test.cpp
	$(CPP) $(CFLAGS) -o $@ $^ $(INCLUDEDIR) $(LIBS)

lock_free_hash_table_test: $(TESTDIR)/lock_free_hash_table_test.cpp
	$(CPP) $(CFLAGS) -o $@ $^ $(INCLUDEDIR) $(LIBS)

clean:
	rm -rf $(PROGRAMS) *.o *.a a.out *.err *~