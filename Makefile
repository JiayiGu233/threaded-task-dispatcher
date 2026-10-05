CC ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra
CPPFLAGS ?=
LDLIBS = -pthread -lcrypto
all: bin/hogwarts
bin/hogwarts: hogwarts.c
	mkdir -p bin
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)
clean:
	rm -rf bin
.PHONY: all clean
