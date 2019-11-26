TOPDIR  := $(shell if [ "$$PWD" != "" ] ; then echo $$PWD ; else pwd ; fi)

C_SOURCES = $(wildcard main.c)
HEADERS = $(wildcard *.h)

# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o}

CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld
GDB = /usr/local/bin/i386-elf-gdb

# -g: Use debugging symbols in gcc
CFLAGS = -g -std=gnu99 -ffreestanding -Wall -Wextra

ui: ${OBJ}
	${CC} -o $@ -T linker.ld $^ -ffreestanding -nostdlib -lgcc -g

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.bin *.o *.elf
	rm -rf *.o **/*.o