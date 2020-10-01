TOPDIR  := $(shell if [ "$$PWD" != "" ] ; then echo $$PWD ; else pwd ; fi)
INCLUDE = $(TOPDIR)/../../

C_SOURCES = $(wildcard window_server.c src/*.c ../../libraries/**/*.c ../../libraries/**/**/*.c)
HEADERS = $(wildcard *.h src/*.h ../../libraries/**/*.h ../../libraries/**/**/*.h)

# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o}

CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld
GDB = /usr/local/bin/i386-elf-gdb

# -g: Use debugging symbols in gcc
CFLAGS = -g -std=gnu18 -ffreestanding -Wall -Wextra -Wno-unused-parameter -Wno-discarded-qualifiers -Wno-comment -Wno-multichar -Wno-sequence-point -Wno-unused-function -Wno-unused-value -I$(INCLUDE)/libraries/libc -I$(INCLUDE)/libraries

window_server: ${OBJ}
	${CC} -o $@ -T linker.ld $^ -ffreestanding -nostdlib -lgcc -g

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.bin *.o *.elf
	rm -rf *.o **/*.o
