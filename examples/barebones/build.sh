rm *.o
rm *.bin
rm *.ios

nasm -f elf32 boot.s -o boot.o
i386-elf-gcc -c kernel.c -o kernel.o -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i386-elf-gcc -c DebugDisplay.c -o DebugDisplay.o -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i386-elf-gcc -c string.c -o string.o -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i386-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o DebugDisplay.o string.o -lgcc

if i386-elf-grub-file --is-x86-multiboot myos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi

mkdir -p isodir/boot/grub
cp myos.bin isodir/boot/myos.bin
cp grub.cfg isodir/boot/grub/grub.cfg
i386-elf-grub-mkrescue -o myos.iso isodir

# qemu-system-i386 -cdrom myos.iso
bochs -f .bochsrc -q

# For debug
# qemu-system-i386 -s -S -kernel myos.bin
# Open another terminal:
#   gdb myos.bin
#   target remote localhost:1234 
#   break kernel_main