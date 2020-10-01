ROOTDIR := $(shell cd ../.. && pwd)

C_SOURCES = $(wildcard calculator.c src/*.c $(ROOTDIR)/libraries/**/*.c $(ROOTDIR)/libraries/**/**/*.c)
HEADERS = $(wildcard *.h src/*.h $(ROOTDIR)/libraries/**/*.h $(ROOTDIR)/libraries/**/**/*.h)

# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o}

CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld
GDB = /usr/local/bin/i386-elf-gdb

# -g: Use debugging symbols in gcc
CFLAGS = -g -std=gnu18 -ffreestanding -Wall -Wextra -Wno-unused-parameter -Wno-discarded-qualifiers -Wno-comment -Wno-multichar -Wno-sequence-point -Wno-unused-function -Wno-unused-value -I$(ROOTDIR)/libraries/libc -I$(ROOTDIR)/libraries

calculator: ${OBJ}
	${CC} -o $@ -T linker.ld $^ -ffreestanding -nostdlib -lgcc -g

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -rf *.bin *.o *.elf
	rm -rf *.o **/*.o
