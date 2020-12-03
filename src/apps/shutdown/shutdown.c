#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static __inline void datawriteportb(unsigned short _port, unsigned char _data)
{
    asm volatile("outb %1, %0"
                 :
                 : "dN"(_port), "a"(_data));
}
int main()
{
    printf("Shutting down the system...");
    datawriteportb(0x501, 0x45);
    return 0;
}