CC := gcc
CFLAGS := -static-pie -lcapstone
LDFLAGS :=
SOURCES := sdb.c
OBJECTS := $(SOURCES:.c=.o)
EXECUTABLE := sdb

.PHONY: all clean

all:
	g++ -o sdb sdb.cpp $(CFLAGS)

clean:
	rm -f sdb
