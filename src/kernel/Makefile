TOPDIR  := $(shell if [ "$$PWD" != "" ] ; then echo $$PWD ; else pwd ; fi)
INCLUDE = $(TOPDIR)/../

C_SOURCES = $(wildcard main.c *.c include/*.c graphics/*.c memory/*.c cpu/*.c devices/*.c system/*.c fs/*.c fs/**/*.c fonts/*.c)
HEADERS = $(wildcard *.h include/*.h graphics/*.h memory/*.h cpu/*.h devices/*.h system/*.h fs/*.h fs/**/*.h fonts/*.h)

# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o boot.o cpu/interrupt.o cpu/descriptor.o system/scheduler.o system/user_handler.o}

CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld
GDB = /usr/local/bin/i386-elf-gdb

# -g: Use debugging symbols in gcc
CFLAGS = -g -std=gnu99 -ffreestanding -Wall -Wextra -I$(INCLUDE)

kernel.bin: ${OBJ}
	${CC} -o $@ -T linker.ld $^ -ffreestanding -nostdlib -lgcc -g

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm -f elf32 -F dwarf -g -O0 $< -o $@ 

clean:
	rm -rf *.bin *.o *.elf
	rm -rf *.o **/*.o