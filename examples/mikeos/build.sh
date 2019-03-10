rm -rf ./Stage1/Boot1.bin #remove without confirmation
rm -rf ./Stage2/KRNLDR.SYS
rm -rf ./Kernel/KRNL.SYS
rm -rf ./floppy.img
rm -rf ./tmp-loop

cd Stage1 && nasm -f bin -o Boot1.bin Boot1.asm && cd ../
cd Stage2 && nasm -f bin -o KRNLDR.SYS Stage2.asm && cd ../
cd kernel && make clean && make && cd ../
dd if=/dev/zero bs=512 count=2880 > floppy.img
dd conv=notrunc if=Stage1/Boot1.bin of=floppy.img
dev=`hdid -nobrowse -nomount floppy.img` #-nomount similar to -mount suppressed
mkdir tmp-loop && mount -t msdos ${dev} tmp-loop && cp ./Stage2/KRNLDR.SYS tmp-loop && cp ./kernel/KRNL.SYS tmp-loop
#mount is used to mount file systems -t is used to indicate the file system type
#so here our type is msdos (FAT12) right?
diskutil unmount tmp-loop
echo 'unmounted folder'
hdiutil detach ${dev}
rm -rf ./tmp-loop
echo 'done'
qemu-system-i386 -boot a -drive format=raw,file=./floppy.img,index=0,if=floppy -d guest_errors,int