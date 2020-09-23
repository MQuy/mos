TOPDIR  := $(shell if [ "$$PWD" != "" ] ; then echo $$PWD ; else pwd ; fi)
INCLUDE = $(TOPDIR)/../

C_SOURCES = $(wildcard main.c *.c ../include/*.c utils/*.c utils/**/*.c memory/*.c cpu/*.c devices/*.c devices/**/*.c system/*.c fs/*.c fs/**/*.c proc/*.c locking/*.c ipc/*.c net/*.c net/devices/*.c)
HEADERS = $(wildcard *.h ../include/*.h utils/*.h memory/*.h cpu/*.h devices/*.h devices/**/*.h system/*.h fs/*.h fs/**/*.h proc/*.h locking/*.h ipc/*.h net/*.h net/devices/*.h)

# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o boot.o cpu/interrupt.o cpu/descriptor.o proc/scheduler.o proc/user.o}

CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld
GDB = /usr/local/bin/i386-elf-gdb

# -g: Use debugging symbols in gcc
CFLAGS = -g -std=gnu18 -ffreestanding -Wall -Wextra -Wno-unused-parameter -Wno-discarded-qualifiers -Wno-comment -Wno-multichar -Wno-sequence-point -Wno-switch -Wno-unused-function -Wno-unused-value -Wno-sign-compare -I$(INCLUDE)

kernel.bin: ${OBJ}
	${CC} -o $@ -T linker.ld $^ -ffreestanding -nostdlib -lgcc -g

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm -f elf32 -F dwarf -g -O0 $< -o $@ 

clean:
	rm -rf *.bin *.o *.elf
	rm -rf *.o **/*.o
