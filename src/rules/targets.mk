${OUTPUT}: ${OBJ}
	${CC} -o $@ -T linker.ld $^ -ffreestanding -nostdlib -lgcc -g

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm -f elf32 -F dwarf -g -O0 $< -o $@

clean:
	rm -rf *.bin *.o *.elf
	rm -rf *.o **/*.o
