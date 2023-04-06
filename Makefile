CC      = gcc
CFLAGS	= -O3 -march=native -mtune=native -fopenmp
WFLAGS	= -std=c11 -Wall -Wextra -g
LDFLAGS	= -lm

TARGETS	= tiny_md viz
SOURCES	= $(shell echo *.c)
OBJECTS = core.o wtime.o

all: $(TARGETS)

viz: viz.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lGL -lGLU -lglut

tiny_md: tiny_md.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(WFLAGS) $(CPPFLAGS) $(CFLAGS) -c $<

clean:
	rm -f $(TARGETS) *.o *.xyz *.log .depend

.depend: $(SOURCES)
	$(CC) -MM $^ > $@

test: tiny_md
	python3 test/test_main.py

-include .depend

.PHONY: clean all test
