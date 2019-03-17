C_SOURCES = $(wildcard kernel.c *.c Include/*.c Hal/*.c)
HEADERS = $(wildcard *.h Include/*.h Hal/*.h)
# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o}

# Change this if your cross-compiler is somewhere else

CC = /usr/local/bin/i386-elf-gcc
LD = /usr/local/bin/i386-elf-ld
# -g: Use debugging symbols in gcc
CFLAGS = -g

# '--oformat binary' deletes all symbols as a collateral, so we don't need
# to 'strip' them manually on this case
KRNL.SYS: ${OBJ}
	${LD} -o $@ -T link.ld $^ --oformat binary

# Generic rules for wildcards
# To make an object, always compile from its .c
%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	rm -rf *.bin *.o *.elf *.SYS
	rm -rf *.o **/*.o