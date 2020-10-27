ROOTDIR := $(shell cd .. && pwd)

C_SOURCES = $(filter-out crt0.c, $(wildcard *.c **/*.c))
A_SOURCES = $(wildcard *.s **/*.s)
HEADERS = $(wildcard *.h **/*.h)

# Nice syntax for file extension replacement
LIBC_OBJ = ${A_SOURCES:.s=.o} ${C_SOURCES:.c=.o}

CC = i386-mos-gcc
AR = i386-mos-ar

# -g: Use debugging symbols in gcc
CFLAGS= -g -std=gnu99 -Wall -Wextra -Wno-unused-parameter -Wno-discarded-qualifiers -Wno-comment -Wno-multichar -Wno-sequence-point -Wno-switch -Wno-unused-function -Wno-unused-value -Wno-sign-compare -Wno-implicit-fallthrough

all: crt0.o libc.a

libc.a: $(LIBC_OBJ)
	$(AR) rcs $@ $(LIBC_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

crt0.o: crt0.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.bin *.o *.elf
	rm -rf *.o **/*.o
