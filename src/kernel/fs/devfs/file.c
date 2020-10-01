#include <fs/vfs.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <shared/ctype.h>
#include <shared/errno.h>
#include <utils/string.h>

#include "devfs.h"

struct vfs_file_operations devfs_dir_operations = {
	.readdir = generic_memory_readdir,
};
