#include <include/mman.h>
#include <include/msgui.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cpu/exception.h"
#include "cpu/gdt.h"
#include "cpu/hal.h"
#include "cpu/idt.h"
#include "cpu/pit.h"
#include "cpu/rtc.h"
#include "cpu/tss.h"
#include "devices/ata.h"
#include "devices/char/memory.h"
#include "devices/kybrd.h"
#include "devices/mouse.h"
#include "devices/pci.h"
#include "devices/serial.h"
#include "fs/ext2/ext2.h"
#include "fs/vfs.h"
#include "ipc/message_queue.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "multiboot2.h"
#include "net/devices/rtl8139.h"
#include "net/dhcp.h"
#include "net/dns.h"
#include "net/icmp.h"
#include "net/net.h"
#include "net/tcp.h"
#include "proc/task.h"
#include "system/framebuffer.h"
#include "system/sysapi.h"
#include "system/time.h"
#include "system/timer.h"
#include "system/uiserver.h"
#include "utils/math.h"
#include "utils/printf.h"
#include "utils/string.h"

extern struct thread *current_thread;
extern struct vfs_file_system_type ext2_fs_type;

void setup_window_server(struct Elf32_Layout *elf_layout)
{
	uiserver_init(current_thread);
	mq_open(WINDOW_SERVER_SHM, 0);

	struct framebuffer *fb = get_framebuffer();
	uint32_t screen_size = fb->height * fb->pitch;
	struct vm_area_struct *area = get_unmapped_area(0, screen_size);
	uint32_t blocks = (area->vm_end - area->vm_start) / PMM_FRAME_SIZE;
	for (uint32_t iblock = 0; iblock < blocks; ++iblock)
		vmm_map_address(
			current_thread->parent->pdir,
			area->vm_start + iblock * PMM_FRAME_SIZE,
			fb->addr + iblock * PMM_FRAME_SIZE,
			I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);

	elf_layout->stack -= sizeof(struct framebuffer);
	struct framebuffer *ws_fb = (struct framebuffer *)elf_layout->stack;
	memcpy(ws_fb, fb, sizeof(struct framebuffer));
	ws_fb->addr = area->vm_start;
}

void client_demo()
{
	int fd = sys_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct socket *sock = sockfd_lookup(fd);
	struct net_device *dev = get_current_net_device();

	struct sockaddr_in *sin = kcalloc(1, sizeof(struct sockaddr_in));
	sin->sin_addr = dev->local_ip;
	sin->sin_port = 40001;
	sock->ops->bind(sock, (struct sockaddr *)sin, sizeof(struct sockaddr_in));

	struct sockaddr_in *din = kcalloc(1, sizeof(struct sockaddr_in));
	// 192.168.1.71
	din->sin_addr = 0xC0A80147;
	din->sin_port = 40000;
	debug_println(DEBUG_INFO, "tcp connecting");
	while (sock->ops->connect(sock, (struct sockaddr *)din, sizeof(struct sockaddr_in)) < 0)
		;

	// debug_println(DEBUG_INFO, "tcp connected");
	// sock->ops->shutdown(sock);
	// debug_println(DEBUG_INFO, "tcp shutdown");

	char message[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Sapien faucibus et molestie ac feugiat sed lectus. Diam volutpat commodo sed egestas egestas fringilla phasellus faucibus. Ultrices in iaculis nunc sed augue. Eget mauris pharetra et ultrices neque ornare aenean euismod. Morbi quis commodo odio aenean sed adipiscing diam donec. Posuere ac ut consequat semper viverra. Facilisi nullam vehicula ipsum a arcu cursus vitae congue. Convallis aenean et tortor at risus viverra adipiscing. Ut enim blandit volutpat maecenas volutpat blandit aliquam. Lorem ipsum dolor sit amet consectetur adipiscing elit. Nec feugiat in fermentum posuere urna nec tincidunt praesent semper. Netus et malesuada fames ac turpis egestas sed tempus. Sed tempus urna et pharetra. Dignissim diam quis enim lobortis. Libero nunc consequat interdum varius sit amet mattis. Cras sed felis eget velit aliquet sagittis id. Nullam vehicula ipsum a arcu cursus vitae congue mauris. Dolor sed viverra ipsum nunc aliquet bibendum. Sollicitudin tempor id eu nisl nunc mi ipsum. Auctor eu augue ut lectus arcu bibendum. Pretium vulputate sapien nec sagittis aliquam malesuada. Aliquam nulla facilisi cras fermentum odio eu feugiat. Sit amet cursus sit amet dictum. Diam vel quam elementum pulvinar etiam non quam. Mi tempus imperdiet nulla malesuada pellentesque elit eget gravida cum. Aenean pharetra magna ac placerat vestibulum. Quam id leo in vitae turpis massa sed elementum tempus. At tempor commodo ullamcorper a. Euismod elementum nisi quis eleifend quam adipiscing vitae proin. Morbi tempus iaculis urna id volutpat. Diam sollicitudin tempor id eu nisl nunc mi ipsum faucibus. Penatibus et magnis dis parturient montes nascetur ridiculus mus. Ut tortor pretium viverra suspendisse potenti nullam ac tortor vitae. Mauris pellentesque pulvinar pellentesque habitant. Turpis nunc eget lorem dolor sed viverra. Sociis natoque penatibus et magnis dis parturient. Malesuada pellentesque elit eget gravida cum sociis. Sapien eget mi proin sed libero enim. Commodo elit at imperdiet dui accumsan. Ornare massa eget egestas purus. Nunc vel risus commodo viverra maecenas. Elementum integer enim neque volutpat. Amet facilisis magna etiam tempor orci eu. Pellentesque sit amet porttitor eget dolor morbi non arcu. Orci porta non pulvinar neque laoreet suspendisse interdum consectetur. Leo vel orci porta non. Fames ac turpis egestas maecenas pharetra. Vestibulum sed arcu non odio euismod lacinia at quis risus. Bibendum arcu vitae elementum curabitur vitae. Tellus id interdum velit laoreet id donec ultrices tincidunt arcu. Velit sed ullamcorper morbi tincidunt ornare massa eget egestas. Consectetur libero id faucibus nisl tincidunt eget. Tortor consequat id porta nibh venenatis cras sed. At elementum eu facilisis sed odio morbi quis commodo. Sit amet venenatis urna cursus. Sapien nec sagittis aliquam malesuada bibendum arcu. Pretium lectus quam id leo in vitae turpis. Augue interdum velit euismod in pellentesque massa placerat duis ultricies. Rhoncus dolor purus non enim praesent. Faucibus purus in massa tempor nec feugiat nisl pretium. Risus pretium quam vulputate dignissim suspendisse in est. Volutpat commodo sed egestas egestas fringilla phasellus faucibus. Tellus integer feugiat scelerisque varius morbi. Ultrices gravida dictum fusce ut placerat orci nulla pellentesque. Et odio pellentesque diam volutpat commodo. Pellentesque eu tincidunt tortor aliquam nulla facilisi cras fermentum odio. Sit amet aliquam id diam maecenas ultricies mi eget mauris. Tristique senectus et netus et malesuada fames ac turpis. Eget sit amet tellus cras adipiscing enim eu turpis. Ridiculus mus mauris vitae ultricies leo. Scelerisque fermentum dui faucibus in ornare quam viverra. Pellentesque dignissim enim sit amet venenatis. Egestas sed sed risus pretium. Malesuada bibendum arcu vitae elementum. Quam adipiscing vitae proin sagittis. Pretium viverra suspendisse potenti nullam ac. Habitasse platea dictumst quisque sagittis purus sit amet. Facilisis leo vel fringilla est ullamcorper eget nulla facilisi etiam. Eu non diam phasellus vestibulum lorem sed risus. In mollis nunc sed id semper risus. Nunc sed blandit libero volutpat sed. Vestibulum sed arcu non odio euismod lacinia at. Pellentesque dignissim enim sit amet. Gravida cum sociis natoque penatibus et magnis. Enim ut sem viverra aliquet eget sit amet. Amet nisl suscipit adipiscing bibendum est ultricies integer quis. Fringilla urna porttitor rhoncus dolor purus. Et odio pellentesque diam volutpat commodo sed egestas. Viverra accumsan in nisl nisi scelerisque. Netus et malesuada fames ac turpis egestas. Vulputate ut pharetra sit amet aliquam. Nunc sed augue lacus viverra vitae congue. Aliquet porttitor lacus luctus accumsan tortor. Nunc consequat interdum varius sit amet mattis vulputate enim. Tincidunt nunc pulvinar sapien et ligula ullamcorper malesuada. In mollis nunc sed id semper risus. Diam vel quam elementum pulvinar etiam non quam lacus suspendisse. At urna condimentum mattis pellentesque id nibh tortor id. Amet massa vitae tortor condimentum lacinia quis.";
	sock->ops->sendmsg(sock, message, sizeof(message));
	debug_println(DEBUG_INFO, "tcp sending done");
	sock->ops->shutdown(sock);
}

void kernel_init()
{
	timer_init();

	// setup random's seed
	srand(get_seconds(NULL));

	// FIXME: MQ 2019-11-19 ata_init is not called in pci_scan_buses without enabling -O2
	pci_init();
	ata_init();

	vfs_init(&ext2_fs_type, "/dev/hda");
	chrdev_memory_init();

	net_init();
	rtl8139_init();
	dhcp_setup();

	client_demo();

	// init ipc message queue
	mq_init();

	// register system apis
	syscall_init();

	// process_load("window server", "/bin/window_server", 0, setup_window_server);

	// idle
	update_thread(current_thread, THREAD_WAITING);
	schedule();

	for (;;)
		;
}

int kernel_main(unsigned long addr, unsigned long magic)
{
	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
		return -1;

	struct multiboot_tag_basic_meminfo *multiboot_meminfo;
	struct multiboot_tag_mmap *multiboot_mmap;
	struct multiboot_tag_framebuffer *multiboot_framebuffer;

	struct multiboot_tag *tag;
	for (tag = (struct multiboot_tag *)(addr + 8);
		 tag->type != MULTIBOOT_TAG_TYPE_END;
		 tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
	{
		switch (tag->type)
		{
		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
			multiboot_meminfo = (struct multiboot_tag_basic_meminfo *)tag;
			break;
		case MULTIBOOT_TAG_TYPE_MMAP:
		{
			multiboot_mmap = (struct multiboot_tag_mmap *)tag;
			break;
		}
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
		{
			multiboot_framebuffer = (struct multiboot_tag_framebuffer *)tag;
			break;
		}
		}
	}

	// setup serial ports
	serial_init();

	// gdt including kernel, user and tss
	gdt_init();
	install_tss(5, 0x10, 0);

	// register irq and handlers
	idt_init();

	// physical memory and paging
	pmm_init(multiboot_meminfo, multiboot_mmap);
	vmm_init();

	exception_init();

	// timer and keyboard
	rtc_init();
	pit_init();
	kkybrd_install();
	mouse_init();

	framebuffer_init(multiboot_framebuffer);

	// enable interrupts to start irqs (timer, keyboard)
	enable_interrupts();

	task_init(kernel_init);

	for (;;)
		;

	return 0;
}
