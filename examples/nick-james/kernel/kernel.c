#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "../memory/kheap.h"
#include "../memory/paging.h"
#include <stdint.h>

typedef struct
{

    uint32_t startLo;
    uint32_t startHi;
    uint32_t sizeLo;
    uint32_t sizeHi;
    uint32_t type;
    uint32_t acpi_3_0;
} memory_region;

char *strMemoryTypes[] = {

    {"Available"},      //memory_region.type==0
    {"Reserved"},       //memory_region.type==1
    {"ACPI Reclaim"},   //memory_region.type==2
    {"ACPI NVS Memory"} //memory_region.type==3
};

void kernel_main()
{
    isr_install();
    irq_install();
    initialise_paging();

    region_c();
}

void fail_segment()
{
    uint32_t *ptr = (uint32_t *)0x1000000;
    uint32_t do_page_fault = *ptr;

    fprint("Type something, it will go through the kernel\n"
           "Type END to halt the CPU or PAGE to request a kmalloc()\n> ");
}

void region_c()
{
    memory_region *region = (memory_region *)0x1000;
    const int smap_size = 0x2000;
    const max_entries = smap_size / sizeof(memory_region);

    for (int i = 0; i < max_entries; ++i)
    {

        if (region[i].type > 4)
            region[i].type = 1;

        if (i > 0 && region[i].startLo == 0)
            break;

        fprint("region %i: start: 0x%x %x length (bytes): 0x%x %x type: %i (%s)\n", i,
               region[i].startHi, region[i].startLo,
               region[i].sizeHi, region[i].sizeLo,
               region[i].type, strMemoryTypes[region[i].type - 1]);
    }
}

typedef struct SMAP_entry
{

    uint32_t BaseL; // base address uint64_t
    uint32_t BaseH;
    uint32_t LengthL; // length uint64_t
    uint32_t LengthH;
    uint32_t Type; // entry Type
    uint32_t ACPI; // extended

} __attribute__((packed)) SMAP_entry_t;

// load memory map to buffer - note: regparm(3) avoids stack issues with gcc in real mode
int __attribute__((noinline)) __attribute__((regparm(3))) detectMemory(SMAP_entry_t *buffer, int maxentries)
{
    uint32_t contID = 0;
    int entries = 0, signature, bytes;
    do
    {
        __asm__ __volatile__("int  $0x15"
                             : "=a"(signature), "=c"(bytes), "=b"(contID)
                             : "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(buffer));
        if (signature != 0x534D4150)
            return -1; // error
        if (bytes > 20 && (buffer->ACPI & 0x0001) == 0)
        {
            // ignore this entry
        }
        else
        {
            buffer++;
            entries++;
        }
    } while (contID != 0 && entries < maxentries);
    return entries;
}

void region_asm()
{
    SMAP_entry_t *smap = (SMAP_entry_t *)0x1000;
    const int smap_size = 0x2000;

    int entry_count = detectMemory(smap, smap_size / sizeof(SMAP_entry_t));

    if (entry_count == -1)
    {
        fprint("halt system and/or show error message");
    }
    else
    {
        fprint("halt system and/or show error message");
        SMAP_entry_t *buffer = smap;
        for (int i = 0; i < entry_count; ++i)
        {
            fprint("region %i: start: 0x%x%x length (bytes): 0x%x%x type: %i (%s)\n", i + 1,
                   buffer->BaseH, buffer->BaseL,
                   buffer->LengthH, buffer->LengthL,
                   buffer->Type, strMemoryTypes[buffer->Type - 1]);
            buffer++;
        }
    }
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
