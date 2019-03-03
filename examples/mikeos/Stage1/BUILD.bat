
nasm -f bin Boot1.asm -o Boot1.bin

PARTCOPY Boot1.bin 0 3 -f0 0 
PARTCOPY Boot1.bin 3E 1C2 -f0 3E 

pause
