#include <fs/vfs.h>

#include "devfs.h"

struct vfs_file_operations devfs_dir_operations = {
	.readdir = generic_memory_readdir,
};
