ROOTDIR := $(shell cd ../.. && pwd)

C_SOURCES = $(wildcard *.c src/*.c src/**/*.c $(ROOTDIR)/libraries/**/*.c $(ROOTDIR)/libraries/**/**/*.c)
A_SOURCES = $(wildcard *.S src/*.S src/**/*.c $(ROOTDIR)/libraries/**/*.S $(ROOTDIR)/libraries/**/**/*.S)
HEADERS = $(wildcard *.h src/*.h src/**/*.h $(ROOTDIR)/libraries/**/*.h $(ROOTDIR)/libraries/**/**/*.h)

# Nice syntax for file extension replacement
OBJ =${A_SOURCES:.S=.o} ${C_SOURCES:.c=.o}

# -g: Use debugging symbols in gcc
CFLAGS = -g -std=gnu18 -ffreestanding -Wall -Wextra -Wno-unused-parameter -Wno-discarded-qualifiers -Wno-comment -Wno-multichar -Wno-sequence-point -Wno-unused-function -Wno-unused-value -Wno-sign-compare -Wno-implicit-fallthrough -I$(ROOTDIR)/libraries/libc -I$(ROOTDIR)/libraries
