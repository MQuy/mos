rm *.bin
rm *.iso

cd kernel && make clean && make && cp kernel.bin ../mos.bin && cd ../

if i386-elf-grub-file --is-x86-multiboot mos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi

mkdir -p isodir/boot/grub
cp mos.bin isodir/boot/mos.bin && rm mos.bin
cp grub.cfg isodir/boot/grub/grub.cfg
i386-elf-grub-mkrescue -o mos.iso isodir

bochs -f .bochsrc -q -rc .debugrc