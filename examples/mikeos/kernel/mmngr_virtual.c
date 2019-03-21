
//****************************************************************************
//**
//**    mmngr_virtual.cpp
//**		-Virtual Memory Manager
//**
//****************************************************************************

//============================================================================
//    IMPLEMENTATION HEADERS
//============================================================================

#include "./Include/string.h"
#include "mmngr_virtual.h"
#include "mmngr_phys.h"

//============================================================================
//    IMPLEMENTATION PRIVATE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

//! page table represents 4mb address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000

//! directory table represents 4gb address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

//! page sizes are 4k
#define PAGE_SIZE 4096

//============================================================================
//    IMPLEMENTATION PRIVATE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE STRUCTURES / UTILITY CLASSES
//============================================================================
//============================================================================
//    IMPLEMENTATION REQUIRED EXTERNAL REFERENCES (AVOID)
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE DATA
//============================================================================

//! current directory table
pdirectory *_cur_directory = 0;

//! current page directory base register
physical_addr _cur_pdbr = 0;

//============================================================================
//    INTERFACE DATA
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTION PROTOTYPES
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTIONS
//============================================================================
//============================================================================
//    INTERFACE FUNCTIONS
//============================================================================

pt_entry *vmmngr_ptable_lookup_entry(ptable *p, virtual_addr addr)
{

   if (p)
      return &p->m_entries[PAGE_TABLE_INDEX(addr)];
   return 0;
}

pd_entry *vmmngr_pdirectory_lookup_entry(pdirectory *p, virtual_addr addr)
{

   if (p)
      return &p->m_entries[PAGE_TABLE_INDEX(addr)];
   return 0;
}

bool vmmngr_switch_pdirectory(pdirectory *dir)
{

   if (!dir)
      return false;

   _cur_directory = dir;
   pmmngr_load_PDBR(_cur_pdbr);
   return true;
}

void vmmngr_flush_tlb_entry(virtual_addr addr)
{
   __asm__ __volatile__("cli        \n\t"
                        "invlpg %0  \n\t"
                        "sti        \n\t"
                        :
                        : "m"(addr)
                        :);
}

pdirectory *vmmngr_get_directory()
{

   return _cur_directory;
}

bool vmmngr_alloc_page(pt_entry *e)
{

   //! allocate a free physical frame
   void *p = pmmngr_alloc_block();
   if (!p)
      return false;

   //! map it to the page
   pt_entry_set_frame(e, (physical_addr)p);
   pt_entry_add_attrib(e, I86_PTE_PRESENT);
   //doesent set WRITE flag...

   return true;
}

void vmmngr_free_page(pt_entry *e)
{

   void *p = (void *)pt_entry_pfn(*e);
   if (p)
      pmmngr_free_block(p);

   pt_entry_del_attrib(e, I86_PTE_PRESENT);
}

void vmmngr_map_page(void *phys, void *virt)
{

   //! get page directory
   pdirectory *pageDirectory = vmmngr_get_directory();

   //! get page table
   pd_entry *e = &pageDirectory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];
   if ((*e & I86_PTE_PRESENT) != I86_PTE_PRESENT)
   {

      //! page table not present, allocate it
      ptable *table = (ptable *)pmmngr_alloc_block();
      if (!table)
         return;

      //! clear page table
      memset(table, 0, sizeof(ptable));

      //! create a new entry
      pd_entry *entry =
          &pageDirectory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

      //! map in the table (Can also just do *entry |= 3) to enable these bits
      pd_entry_add_attrib(entry, I86_PDE_PRESENT);
      pd_entry_add_attrib(entry, I86_PDE_WRITABLE);
      pd_entry_set_frame(entry, (physical_addr)table);
   }

   //! get table
   ptable *table = (ptable *)PAGE_GET_PHYSICAL_ADDRESS(e);

   //! get page
   pt_entry *page = &table->m_entries[PAGE_TABLE_INDEX((uint32_t)virt)];

   //! map it in (Can also do (*page |= 3 to enable..)
   pt_entry_set_frame(page, (physical_addr)phys);
   pt_entry_add_attrib(page, I86_PTE_PRESENT);
}

void vmmngr_initialize()
{

   //! allocate default page table
   ptable *table = (ptable *)pmmngr_alloc_block();
   if (!table)
      return;

   //! allocates 3gb page table
   ptable *table2 = (ptable *)pmmngr_alloc_block();
   if (!table2)
      return;

   //! clear page table
   memset(table, 0, sizeof(ptable));

   //! 1st 4mb are idenitity mapped
   for (int i = 0, frame = 0x0, virt = 0x00000000; i < 1024; i++, frame += 4096, virt += 4096)
   {

      //! create a new page
      pt_entry page = 0;
      pt_entry_add_attrib(&page, I86_PTE_PRESENT);
      pt_entry_set_frame(&page, frame);

      //! ...and add it to the page table
      table2->m_entries[PAGE_TABLE_INDEX(virt)] = page;
   }

   //! map 1mb to 3gb (where we are at)
   for (int i = 0, frame = 0x100000, virt = 0xc0000000; i < 1024; i++, frame += 4096, virt += 4096)
   {

      //! create a new page
      pt_entry page = 0;
      pt_entry_add_attrib(&page, I86_PTE_PRESENT);
      pt_entry_set_frame(&page, frame);

      //! ...and add it to the page table
      table->m_entries[PAGE_TABLE_INDEX(virt)] = page;
   }

   //! create default directory table
   pdirectory *dir = (pdirectory *)pmmngr_alloc_blocks(3);
   if (!dir)
      return;

   //! clear directory table and set it as current
   memset(dir, 0, sizeof(pdirectory));

   //! get first entry in dir table and set it up to point to our table
   pd_entry *entry = &dir->m_entries[PAGE_DIRECTORY_INDEX(0xc0000000)];
   pd_entry_add_attrib(entry, I86_PDE_PRESENT);
   pd_entry_add_attrib(entry, I86_PDE_WRITABLE);
   pd_entry_set_frame(entry, (physical_addr)table);

   pd_entry *entry2 = &dir->m_entries[PAGE_DIRECTORY_INDEX(0x00000000)];
   pd_entry_add_attrib(entry2, I86_PDE_PRESENT);
   pd_entry_add_attrib(entry2, I86_PDE_WRITABLE);
   pd_entry_set_frame(entry2, (physical_addr)table2);

   //! store current PDBR
   _cur_pdbr = (physical_addr)&dir->m_entries;

   //! switch to our page directory
   vmmngr_switch_pdirectory(dir);

   // //! enable paging
   pmmngr_paging_enable(true);
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[mmngr_virtual.cpp]
//**
//****************************************************************************
