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

	char message[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Sit amet tellus cras adipiscing enim eu. Sapien pellentesque habitant morbi tristique senectus et. Nam aliquam sem et tortor consequat id porta nibh. A erat nam at lectus urna duis convallis convallis. Ut sem viverra aliquet eget sit amet tellus. Nullam non nisi est sit amet. Mauris ultrices eros in cursus turpis. Consectetur adipiscing elit ut aliquam. Dui accumsan sit amet nulla facilisi morbi tempus. Eget lorem dolor sed viverra ipsum nunc aliquet bibendum enim. Turpis cursus in hac habitasse platea dictumst quisque. Adipiscing elit duis tristique sollicitudin nibh. Tellus molestie nunc non blandit massa enim nec. Tristique et egestas quis ipsum suspendisse ultrices gravida dictum. Ac tortor vitae purus faucibus ornare suspendisse sed nisi lacus. Feugiat pretium nibh ipsum consequat nisl vel pretium lectus. Dui id ornare arcu odio. Ut consequat semper viverra nam libero. Vitae congue mauris rhoncus aenean vel elit. Mattis vulputate enim nulla aliquet. Ipsum dolor sit amet consectetur adipiscing elit duis tristique. Nulla facilisi cras fermentum odio eu. Porttitor lacus luctus accumsan tortor posuere ac ut consequat. Morbi enim nunc faucibus a pellentesque sit amet. Integer quis auctor elit sed vulputate mi sit amet. Sed enim ut sem viverra aliquet eget. Volutpat blandit aliquam etiam erat velit scelerisque in dictum non. Habitant morbi tristique senectus et netus et malesuada fames ac. Proin sagittis nisl rhoncus mattis rhoncus urna. Risus nec feugiat in fermentum posuere. Facilisis sed odio morbi quis commodo. Quam nulla porttitor massa id neque. Pellentesque elit eget gravida cum. Lobortis scelerisque fermentum dui faucibus. Est ante in nibh mauris cursus mattis molestie. Ornare suspendisse sed nisi lacus sed viverra. Vestibulum sed arcu non odio euismod lacinia at. Eu volutpat odio facilisis mauris sit amet. Curabitur vitae nunc sed velit dignissim sodales ut. Varius quam quisque id diam. Nunc eget lorem dolor sed. Arcu non sodales neque sodales ut etiam. Viverra accumsan in nisl nisi scelerisque. At consectetur lorem donec massa sapien faucibus et molestie. Arcu non odio euismod lacinia at quis. Nunc lobortis mattis aliquam faucibus purus in. Orci phasellus egestas tellus rutrum tellus pellentesque eu tincidunt. Vulputate sapien nec sagittis aliquam. Sit amet luctus venenatis lectus magna fringilla urna porttitor. Aenean vel elit scelerisque mauris pellentesque pulvinar pellentesque. Pulvinar sapien et ligula ullamcorper malesuada proin. Integer eget aliquet nibh praesent tristique magna. Luctus accumsan tortor posuere ac ut consequat semper. Aliquam etiam erat velit scelerisque. Quisque sagittis purus sit amet volutpat consequat. Id semper risus in hendrerit gravida rutrum quisque. Vitae suscipit tellus mauris a diam. Vulputate mi sit amet mauris commodo quis imperdiet massa. Donec pretium vulputate sapien nec sagittis aliquam malesuada bibendum. Suspendisse potenti nullam ac tortor. Massa massa ultricies mi quis. Viverra orci sagittis eu volutpat. Elit ullamcorper dignissim cras tincidunt lobortis feugiat. Nisi lacus sed viverra tellus in hac habitasse platea dictumst. Faucibus in ornare quam viverra orci sagittis eu volutpat. Ipsum dolor sit amet consectetur adipiscing elit pellentesque habitant morbi. At auctor urna nunc id cursus metus. Elementum facilisis leo vel fringilla. Gravida quis blandit turpis cursus in. Dignissim convallis aenean et tortor at risus viverra adipiscing at. Volutpat ac tincidunt vitae semper quis lectus nulla. Sapien nec sagittis aliquam malesuada bibendum. Amet volutpat consequat mauris nunc. Et ligula ullamcorper malesuada proin libero nunc. Commodo elit at imperdiet dui. Non quam lacus suspendisse faucibus interdum. Fusce ut placerat orci nulla pellentesque dignissim enim. Nec ullamcorper sit amet risus nullam eget. Enim praesent elementum facilisis leo vel fringilla. Nunc consequat interdum varius sit amet mattis vulputate enim nulla. Dolor morbi non arcu risus quis varius. Faucibus nisl tincidunt eget nullam non nisi est sit amet. Sodales neque sodales ut etiam. In cursus turpis massa tincidunt. Diam quis enim lobortis scelerisque fermentum dui faucibus in. Lorem ipsum dolor sit amet consectetur adipiscing elit. Proin fermentum leo vel orci porta non. Eu sem integer vitae justo eget magna fermentum iaculis. Orci a scelerisque purus semper eget duis at. Tristique senectus et netus et malesuada fames. Et leo duis ut diam quam nulla porttitor. Tempus iaculis urna id volutpat lacus laoreet. Eget felis eget nunc lobortis mattis aliquam. Nec sagittis aliquam malesuada bibendum arcu vitae elementum. Sit amet nulla facilisi morbi tempus iaculis urna. Suspendisse in est ante in nibh mauris. Nisl nisi scelerisque eu ultrices vitae auctor eu. Nam at lectus urna duis convallis. Tristique magna sit amet purus. Tellus elementum sagittis vitae et leo duis ut diam quam. Ut pharetra sit amet aliquam id diam maecenas. Mauris rhoncus aenean vel elit scelerisque. Vestibulum rhoncus est pellentesque elit ullamcorper dignissim cras tincidunt. Mi proin sed libero enim sed faucibus turpis. Elementum integer enim neque volutpat ac tincidunt vitae semper. Auctor eu augue ut lectus arcu bibendum at varius. Nisl vel pretium lectus quam. Mauris nunc congue nisi vitae. Tellus id interdum velit laoreet id donec. Arcu cursus vitae congue mauris rhoncus aenean vel. Dignissim convallis aenean et tortor at risus viverra. In fermentum posuere urna nec. Turpis massa sed elementum tempus. Senectus et netus et malesuada fames ac turpis egestas integer. Nec ullamcorper sit amet risus nullam eget. Semper eget duis at tellus at urna condimentum mattis. Gravida neque convallis a cras semper. Bibendum neque egestas congue quisque egestas diam. Sem viverra aliquet eget sit amet. Cursus vitae congue mauris rhoncus aenean vel. Velit egestas dui id ornare. Vitae proin sagittis nisl rhoncus mattis rhoncus urna. Risus quis varius quam quisque id. Tincidunt augue interdum velit euismod in pellentesque massa placerat duis. Neque laoreet suspendisse interdum consectetur libero id. Tristique senectus et netus et malesuada fames ac. Mollis aliquam ut porttitor leo a diam sollicitudin tempor. Gravida cum sociis natoque penatibus et. Quis viverra nibh cras pulvinar. Sed libero enim sed faucibus turpis in eu. Nec feugiat in fermentum posuere urna nec tincidunt. Condimentum lacinia quis vel eros donec ac odio tempor orci. Enim nec dui nunc mattis. Feugiat nibh sed pulvinar proin gravida hendrerit lectus a. Lectus magna fringilla urna porttitor rhoncus dolor purus. Augue mauris augue neque gravida in fermentum et sollicitudin ac. Donec enim diam vulputate ut pharetra sit amet aliquam id. Ipsum faucibus vitae aliquet nec ullamcorper sit amet.";
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

int kernel_main(uint32_t addr, uint32_t magic)
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
