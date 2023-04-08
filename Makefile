CC      = gcc
CFLAGS	= -O0
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

test:
	make clean
	make
	python3 test/test_main.py

benchmark:
	make clean
	make
	python3 benchmark/main.py $(name) --n-values 256 500 864 1372 --num-runs 3
	python3 benchmark/main.py $(name) --mode plot

-include .depend

.PHONY: clean all test benchmark
