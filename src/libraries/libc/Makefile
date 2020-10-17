ROOTDIR := $(shell cd .. && pwd)

C_SOURCES = $(wildcard *.c **/*.c)
HEADERS = $(wildcard *.h **/*.h)

# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o}

CC = i386-mos-gcc
AR = i386-mos-ar

# -g: Use debugging symbols in gcc
CFLAGS= -g -std=gnu99 -I. -Iapps -pipe -Wall -Wextra -Wno-unused-parameter -Wno-discarded-qualifiers -Wno-comment -Wno-multichar -Wno-sequence-point -Wno-switch -Wno-unused-function -Wno-unused-value -Wno-sign-compare -Wno-implicit-fallthrough

libc.a: $(OBJ)
	$(AR) cr $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

clean:
	rm -rf *.bin *.o *.elf
	rm -rf *.o **/*.o
