#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "../memory/kheap.h"
#include <stdint.h>

void kernel_main()
{
    isr_install();
    irq_install();

    fprint("Type something, it will go through the kernel\n"
           "Type END to halt the CPU or PAGE to request a kmalloc()\n> ");
}

void user_input(char *input)
{
    if (strcmp(input, "END") == 0)
    {
        fprint("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    }
    else if (strcmp(input, "PAGE") == 0)
    {
        uint32_t phys_addr;
        uint32_t page = kmalloc_ap(1000, &phys_addr);
        fprint("Page: %d, physical address: %x\n", page, phys_addr);
    }
    fprint("You said: %s\n> ", input);
}
