# Makefile for cpuid_features.c
# Usage:
#   make            # build
#   make run        # build + run
#   make clean      # remove build artifacts

TARGET := cpuid_features
SRC    := cpuid_features.c

CC ?= clang
CFLAGS_COMMON := -Wall -Wextra -Wpedantic
CFLAGS_RELEASE := -O2
CFLAGS_DEBUG   := -O0 -g3

UNAME_S := $(shell uname -s)

.PHONY: all run clean debug disasm

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o
