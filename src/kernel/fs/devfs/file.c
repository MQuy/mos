#include <include/ctype.h>
#include <include/errno.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include <kernel/utils/string.h>

#include "devfs.h"

struct vfs_file_operations devfs_dir_operations = {};
