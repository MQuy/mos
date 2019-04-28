C_SOURCES = $(wildcard main.c *.c include/*.c graphics/*.c memory/*.c cpu/*.c devices/*.c system/*.c)
HEADERS = $(wildcard *.h include/*.h graphics/*.h memory/*.h cpu/*.h devices/*.h system/*.h)

# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o cpu/descriptor.o user.o}

CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld
GDB = /usr/local/bin/i386-elf-gdb

# -g: Use debugging symbols in gcc
CFLAGS = -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra

kernel.bin: ${OBJ}
	nasm -f elf32 boot.asm -o boot.o
	${CC} -o $@ -T linker.ld boot.o $^ -ffreestanding -O2 -nostdlib -lgcc 

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@ 

clean:
	rm -rf *.bin *.o *.elf
	rm -rf *.o **/*.o