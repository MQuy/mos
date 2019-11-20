#include "hal.h"
#include <kernel/memory/vmm.h>
#include "apic.h"

const uint32_t CPUID_FLAG_MSR = 1 << 5;
enum
{
  CPUID_FEAT_ECX_SSE3 = 1 << 0,
  CPUID_FEAT_ECX_PCLMUL = 1 << 1,
  CPUID_FEAT_ECX_DTES64 = 1 << 2,
  CPUID_FEAT_ECX_MONITOR = 1 << 3,
  CPUID_FEAT_ECX_DS_CPL = 1 << 4,
  CPUID_FEAT_ECX_VMX = 1 << 5,
  CPUID_FEAT_ECX_SMX = 1 << 6,
  CPUID_FEAT_ECX_EST = 1 << 7,
  CPUID_FEAT_ECX_TM2 = 1 << 8,
  CPUID_FEAT_ECX_SSSE3 = 1 << 9,
  CPUID_FEAT_ECX_CID = 1 << 10,
  CPUID_FEAT_ECX_FMA = 1 << 12,
  CPUID_FEAT_ECX_CX16 = 1 << 13,
  CPUID_FEAT_ECX_ETPRD = 1 << 14,
  CPUID_FEAT_ECX_PDCM = 1 << 15,
  CPUID_FEAT_ECX_PCIDE = 1 << 17,
  CPUID_FEAT_ECX_DCA = 1 << 18,
  CPUID_FEAT_ECX_SSE4_1 = 1 << 19,
  CPUID_FEAT_ECX_SSE4_2 = 1 << 20,
  CPUID_FEAT_ECX_x2APIC = 1 << 21,
  CPUID_FEAT_ECX_MOVBE = 1 << 22,
  CPUID_FEAT_ECX_POPCNT = 1 << 23,
  CPUID_FEAT_ECX_AES = 1 << 25,
  CPUID_FEAT_ECX_XSAVE = 1 << 26,
  CPUID_FEAT_ECX_OSXSAVE = 1 << 27,
  CPUID_FEAT_ECX_AVX = 1 << 28,

  CPUID_FEAT_EDX_FPU = 1 << 0,
  CPUID_FEAT_EDX_VME = 1 << 1,
  CPUID_FEAT_EDX_DE = 1 << 2,
  CPUID_FEAT_EDX_PSE = 1 << 3,
  CPUID_FEAT_EDX_TSC = 1 << 4,
  CPUID_FEAT_EDX_MSR = 1 << 5,
  CPUID_FEAT_EDX_PAE = 1 << 6,
  CPUID_FEAT_EDX_MCE = 1 << 7,
  CPUID_FEAT_EDX_CX8 = 1 << 8,
  CPUID_FEAT_EDX_APIC = 1 << 9,
  CPUID_FEAT_EDX_SEP = 1 << 11,
  CPUID_FEAT_EDX_MTRR = 1 << 12,
  CPUID_FEAT_EDX_PGE = 1 << 13,
  CPUID_FEAT_EDX_MCA = 1 << 14,
  CPUID_FEAT_EDX_CMOV = 1 << 15,
  CPUID_FEAT_EDX_PAT = 1 << 16,
  CPUID_FEAT_EDX_PSE36 = 1 << 17,
  CPUID_FEAT_EDX_PSN = 1 << 18,
  CPUID_FEAT_EDX_CLF = 1 << 19,
  CPUID_FEAT_EDX_DTES = 1 << 21,
  CPUID_FEAT_EDX_ACPI = 1 << 22,
  CPUID_FEAT_EDX_MMX = 1 << 23,
  CPUID_FEAT_EDX_FXSR = 1 << 24,
  CPUID_FEAT_EDX_SSE = 1 << 25,
  CPUID_FEAT_EDX_SSE2 = 1 << 26,
  CPUID_FEAT_EDX_SS = 1 << 27,
  CPUID_FEAT_EDX_HTT = 1 << 28,
  CPUID_FEAT_EDX_TM1 = 1 << 29,
  CPUID_FEAT_EDX_IA64 = 1 << 30,
  CPUID_FEAT_EDX_PBE = 1 << 31
};

bool cpu_has_msr()
{
  uint32_t a, d;
  cpuid(1, &a, &d);
  return d & CPUID_FLAG_MSR;
}

void cpu_get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
  __asm__ __volatile__("rdmsr"
                       : "=a"(*lo), "=d"(*hi)
                       : "c"(msr));
}

void cpu_set_msr(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
  __asm__ __volatile__("wrmsr"
                       :
                       : "a"(lo), "d"(hi), "c"(msr));
}

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

uint32_t apic_base;

bool check_apic()
{
  uint32_t eax, edx;
  cpuid(1, &eax, &edx);
  return edx & CPUID_FEAT_EDX_APIC;
}

/* Set the physical address for local APIC registers */
void cpu_set_apic_base(uintptr_t apic)
{
  uint32_t edx = 0;
  uint32_t eax = (apic & 0xfffff000) | IA32_APIC_BASE_MSR_ENABLE;

  cpu_set_msr(IA32_APIC_BASE_MSR, eax, edx);
}

/**
 * Get the physical address of the APIC registers page
 * make sure you map it to virtual memory ;)
 */
uintptr_t cpu_get_apic_base()
{
  uint32_t eax, edx;
  cpu_get_msr(IA32_APIC_BASE_MSR, &eax, &edx);

  return (eax & 0xfffff000);
}

void apic_write(uint32_t apic_base, uint32_t reg, uint32_t value)
{
  *(volatile uint32_t *)(apic_base + reg) = value;
}

uint32_t apic_read(uint32_t apic_base, uint32_t reg)
{
  return *(volatile uint32_t *)(apic_base + reg);
}

void enable_apic()
{
  uint32_t padd_apic_base = cpu_get_apic_base();
  uint32_t vadd_apic_base = malloc(0x1000);

  vmm_map_phyiscal_address(vmm_get_directory(), vadd_apic_base, padd_apic_base, I86_PTE_PRESENT | I86_PTE_WRITABLE);

  /* Hardware enable the Local APIC if it wasn't enabled */
  cpu_set_apic_base(padd_apic_base);
  /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
  apic_write(apic_base, 0xf0, 0x100 | 0xff);
}