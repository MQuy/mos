#include "pci.h"

#include <cpu/hal.h>
#include <memory/vmm.h>
#include <utils/printf.h>

#include "ata.h"

extern uint32_t current_row, current_column;
static struct list_head ldevs;

static uint32_t pci_get_address(uint8_t bus, uint8_t slot, uint8_t func)
{
	uint32_t lbus = (uint32_t)bus;
	uint32_t lslot = (uint32_t)slot;
	uint32_t lfunc = (uint32_t)func;

	return (lbus << 16) | (lslot << 11) | (lfunc << 8) | 0x80000000;
}

uint32_t pci_read_field(uint32_t address, uint8_t offset)
{
	/* write out the address */
	outportl(PCI_ADDRESS_PORT, address | (offset & 0xfc));
	return inportl(PCI_VALUE_PORT);
}

void pci_write_field(uint32_t address, uint8_t offset, uint32_t value)
{
	outportl(PCI_ADDRESS_PORT, address | (offset & 0xfc));
	outportl(PCI_VALUE_PORT, value);
}

static uint16_t pci_get_device_id(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0x0);
	return (reg >> 16) & 0xFFFF;
}

static uint16_t pci_get_vender_id(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0x0);
	return reg & 0xFFFF;
}

static uint16_t pci_get_status(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0x4);
	return (reg >> 16) & 0xFFFF;
}

uint16_t pci_get_command(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0x4);
	return reg & 0xFFFF;
}

static uint16_t pci_get_class_code(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0x8);
	return (reg >> 24) & 0xFF;
}

static uint16_t pci_get_subclass_code(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0x8);
	return (reg >> 16) & 0xFF;
}

static uint16_t pci_get_prog_if(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0x8);
	return (reg >> 8) & 0xFF;
}

static uint16_t pci_get_header_type(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0xC);
	return (reg >> 16) & 0xFF;
}

static uint16_t pci_get_secondary_bus(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0x18);
	return (reg >> 8) & 0xFF;
}

uint8_t pci_get_interrupt_line(uint32_t address)
{
	uint32_t reg = pci_read_field(address, 0x3C);
	return reg & 0xFF;
}

static void reg_device(uint8_t bus, uint8_t device, uint8_t function)
{
	uint32_t address = pci_get_address(bus, device, function);
	int vendorID = pci_get_vender_id(address);

	if (vendorID != PCI_INVALID_VENDOR_ID)
	{
		int deviceID = pci_get_device_id(address);
		int classCode = pci_get_class_code(address);

		if (classCode != PCI_CLASS_CODE_BRIDGE_DEVICE)
		{
			struct pci_device *dev = kcalloc(1, sizeof(struct pci_device));
			dev->address = address;
			dev->vendorID = vendorID;
			dev->deviceID = deviceID;
			dev->bar0 = pci_read_field(address, PCI_BAR0);

			list_add_tail(&dev->sibling, &ldevs);
		}
	}
}

static void pci_check_function(uint8_t bus, uint8_t device, uint8_t function)
{
	uint8_t secondary_bus;

	reg_device(bus, device, function);

	uint32_t address = pci_get_address(bus, device, function);
	uint16_t baseClass = pci_get_class_code(address);
	uint16_t subClass = pci_get_subclass_code(address);
	if ((baseClass == PCI_CLASS_CODE_BRIDGE_DEVICE) && (subClass == PCI_SUBCLASS_PCI_TO_PCI_BRIDGE))
	{
		secondary_bus = pci_get_secondary_bus(address);
		pci_scan_bus(secondary_bus);
	}
}

void pci_scan_bus(uint8_t bus)
{
	uint8_t device;
	uint8_t function;

	for (device = 0; device < 32; device++)
	{
		uint32_t address = pci_get_address(bus, device, function);
		uint16_t headerType = pci_get_header_type(address);
		if ((headerType & PCI_MULTIFUNCTION_DEVICE) != 0)
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
	int headerType = pci_get_header_type(pci_get_address(0, 0, 0));
	if ((headerType & PCI_MULTIFUNCTION_DEVICE) == 0)
	{
		pci_scan_bus(0);
	}
	else
	{
		uint8_t function, bus;
		for (function = 0; function < 8; function++)
		{
			if (pci_get_vender_id(pci_get_address(0, 0, function)) == PCI_INVALID_VENDOR_ID)
				break;
			bus = function;
			pci_scan_bus(bus);
		}
	}
}

struct pci_device *get_pci_device(int32_t vendorID, int32_t deviceID)
{
	struct pci_device *iter_dev;
	list_for_each_entry(iter_dev, &ldevs, sibling)
	{
		if (iter_dev->vendorID == vendorID && iter_dev->deviceID == deviceID)
			return iter_dev;
	}
	return NULL;
}

void pci_init()
{
	DEBUG &&debug_println(DEBUG_INFO, "PCI: Initializing");

	INIT_LIST_HEAD(&ldevs);
	pci_scan_buses();

	DEBUG &&debug_println(DEBUG_INFO, "PCI: Done");
}
