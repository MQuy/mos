#include <kernel/cpu/hal.h>
#include <kernel/graphics/DebugDisplay.h>
#include "pci.h"
#include "ata.h"

uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  uint16_t tmp = 0;

  /* create configuration address as per Figure 1 */
  address = (uint32_t)((lbus << 16) | (lslot << 11) |
                       (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

  /* write out the address */
  outportl(0xCF8, address);
  /* read in the data */
  /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
  tmp = (uint16_t)((inportl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
  return (tmp);
}

uint16_t get_vender_id(uint16_t bus, uint16_t device, uint16_t function)
{
  return pci_read_word(bus, device, function, 0);
}

uint16_t get_device_id(uint16_t bus, uint16_t device, uint16_t function)
{
  return pci_read_word(bus, device, function, 2);
}

uint16_t get_class_code(uint16_t bus, uint16_t device, uint16_t function)
{
  uint32_t reg = pci_read_word(bus, device, function, 0xA);
  return (reg & 0xFF00) >> 8;
}

uint16_t get_subclass_code(uint16_t bus, uint16_t device, uint16_t function)
{
  uint32_t reg = pci_read_word(bus, device, function, 0xA);
  return (reg & 0x00FF);
}

uint16_t get_prog_if(uint16_t bus, uint16_t device, uint16_t function)
{
  uint32_t reg = pci_read_word(bus, device, function, 0x8);
  return (reg & 0xFF00) >> 8;
}

uint16_t get_header_type(uint16_t bus, uint16_t device, uint16_t function)
{
  uint32_t reg = pci_read_word(bus, device, function, 0xD);
  return (reg & 0x00FF);
}

uint16_t get_secondary_bus(uint16_t bus, uint16_t device, uint16_t function)
{
  uint32_t reg = pci_read_word(bus, device, function, 0x12);
  return (reg & 0xFF00) >> 8;
}

void print_device(uint8_t bus, uint8_t device, uint8_t function)
{
  int vendorID = get_vender_id(bus, device, function);

  if (vendorID != PCI_INVALID_VENDOR_ID)
  {
    int deviceID = get_device_id(bus, device, function);
    int classCode = get_class_code(bus, device, function);
    int subclassCode = get_subclass_code(bus, device, function);
    int progif = get_prog_if(bus, device, function);

    if (classCode == PCI_CLASS_CODE_MASS_STORAGE)
    {
      if (subclassCode == PCI_SUBCLASS_IDE)
        ata_init();
    }
  }
}

void pci_check_function(uint8_t bus, uint8_t device, uint8_t function)
{
  uint8_t secondary_bus;

  print_device(bus, device, function);

  uint16_t baseClass = get_class_code(bus, device, function);
  uint16_t subClass = get_subclass_code(bus, device, function);
  if ((baseClass == PCI_CLASS_CODE_BRIDGE_DEVICE) && (subClass == PCI_SUBCLASS_PCI_TO_PCI_BRIDGE))
  {
    secondary_bus = get_secondary_bus(bus, device, function);
    pci_scan_bus(secondary_bus);
  }
}

void pci_scan_bus(uint8_t bus)
{
  uint8_t device;
  uint8_t function;

  for (device = 0; device < 32; device++)
  {
    uint16_t headerType = get_header_type(bus, device, function);
    if (headerType & PCI_MULTIFUNCTION_DEVICE != 0)
    {
      for (function = 0; function < 7; function++)
      {
        pci_check_function(bus, device, function);
      }
    }
    else
    {
      pci_check_function(bus, device, 0);
    }
  }
}

void pci_scan_buses()
{
  int headerType = get_header_type(0, 0, 0);
  if ((headerType & PCI_MULTIFUNCTION_DEVICE) == 0)
  {
    pci_scan_bus(0);
  }
  else
  {
    uint8_t function, bus;
    for (function = 0; function < 8; function++)
    {
      // FIXME: MQ 2019-05-20 Should it be == instead of !=
      if (get_vender_id(0, 0, function) != PCI_INVALID_VENDOR_ID)
        break;
      bus = function;
      pci_scan_bus(bus);
    }
  }
}