## Linux vfs

### Data structures

There are two kinds of structures:

- those only in memory (for cache purpose)
- others in both (roughly map memory-disk)

#### File

File is created in each process which represents an open file, has important fields below

- dentry: in-memory structure that represents `path`
- mode: read/write permission
- pos: current read/write position
- vfsmount
- file operators

#### Dentry

Dentry is a specific component in a path, has important fields below

- inode
- dparent
- name
- superblock
- subdirs

#### Inode

Inode is an in-memory mapping from a inode in device, has important fields below

- ino: unique number per device
- mode: file/foder/symlink/pipe
- size

#### Superblock

Superblock is an in-memory mapping from a superblock in device, has important fields below

- devname
- blocksize
- droot

### How linux vfs works

#### Open a file

`vfs_open(const char *filename, int flags, int mode)`

- `getname()` to read the file pathname from the process
- `get_unused_fd()` to find an empty slot in `process->files->fd`
- `filp_open()`
  - `open_namei()` creates nameidata structure (dentry, mount), invokes `path_lookup()` check permission and walk through pathname to create dentries if they don't exist
  - `dentry_open()` initializes file structure

`path_lookup(char *name, nameidata *nd)`

- break pathname into components (except the last one), for each component:
  - get the last resolved component (inode, dentry), and the component's name
  - invokes `do_lookup()` to search the dentry of the component in the dentry cache. If not, invokes `real_lookup()` which calls inode's `lookup` to create a new dentry and insert in to dentry cache -> the same for inode.
  - invokes `follow_mount()` to check the component just resolved refers to a mounted-point directory (going through mounted list) to update `next.dentry` and `next.mnt` to mounted filesystem
  - if just resolved component is not a directory, return an error (in middle of original pathname)
- invokes `do_lookup()` to derive the dentry from parent directory and the filename
- invokes `follow_mount()` to update `dentry` and `mnt` in the `next`
- set `nd->mnt` and `nd->dentry` with values in `next`

`inode->lookup(super_block *sb, ino)`

- search for ino in node cache, if found -> return
- invokes `get_new_inode()`
  - calls `sb->alloc_inode()` and `sb->read_inode()`

[More details](https://www.win.tue.nl/~aeb/linux/vfs/trail-2.html)

#### Close a file

`sys_close(fd)`

- get file from fd, release it from array `files->fd[fd]`
- invokes `filp_close()`
  - invokes `file->f_op->flush()`
  - `fput(file)` to release file (`file->f_op->release(inode, file)`)

#### Read a file

`vfs_fread(unsigned int fd, char *buf, size_t count)`

- `fget()` to get file
- invokes `file_pos_read()` to get pos
- `file->f_op->read(file, buf, count, pos)`
  - consider the file as subdivided in pages of data (4096 bytes/page)
  - index = pos >> 12, offset = pos & 0x00000fff
  - for each page (less than count)
    - check index\*4096 + offset > file size
    - invokes `readpage` -> `ext2_readpage`
      - block_in_file = index << (12 - i_blkbits)
      - `ext2_get_block(inode, block_in_file)`
  - update `pos` and `time`

#### Write a file

`vfs_fwrite(unsigned fd, char *buf, size_t count)`

- `fget()` to get file
- invokes `file_pos_read()` to get pos
- `file->f_op->write(file, buf, count, pos)`
  - for each page
    - invokes `__generic_file_aio_write_nolock()`
      - `ext2_get_block()` allocates a new physical block for file and mark it as dirty
    - invokes `writepages()` to flush the page to disk

### EXT2

`ext2_new_inode`, `ext2_free_inode` are only related to inode allocation without working data blocks

#### `ext2_new_inode(dir, mode)`

- `new_inode()` to allocate a new VFS inode
- find block group number
  - if directory
    - Top level: spread among all block groups (prefer block group which have more number of free inodes and blocks)
    - Nested: prefer putting in the group of parent > other block groups and not having too many directories, a sufficient number of free inodes left and small "debts"
  - if file, starting from the parent group -> log(n) groups -> linear groups which have free inodes and blocks
- search for the first null bit in bitmap block, allocate disk node and other related information `like bg_free_inodes_count`, `bg_used_dirs_count` ...

#### `ext2_free_inode(inode)`

is called after cleaning of internal data structures and data in file itself.

- invokes `clear_inode()` which calls sb->clear_inode()
- computes the index of the block group containing the disk inode
- Clear bit in bitmap block and related information like increasing `bg_free_inodes_count`, decreasing `bg_used_dirs_count` ...

#### `ext2_get_block(inode, iblock, create)`

- find iblock via `ext2_get_branch`
- if iblock doesn't exist, allocating block and invokes `ext2_alloc_block(inode, goal)` with goal
  - if iblock is being allocated and previous allocated block have consecutive file block numbers the goal is the iblock
  - prefer goal is on of allocated block numbers that precedes to iblock
  - as the last resort, the goal is the first block number of block group which contains inode

#### `ext2_alloc_block(inode, goal)`

- check the goal is one of the preallocated blocks -> allocating that block
- if not, discarding all remaining preallocated blocks and invoking `ext2_new_block`

**ext2_new_block(inode, goal)**

- if the preferred block is free -> allocating the block
- if the goal is busy, checking one of next blocks after the preferred block are free
- if not, considering all block groups, starting from the one including the goal. Choosing block based on
  - Look for a group of at least eight adjacent free blocks
  - If no such group -> single free block
- tries to preallocate up to eight free blocks adjacent to the free block and set `i_prealloc_block`, `i_prealloc_count`

### `ext2_free_blocks(inode, block, count)`

- get and clean the bitmap block which contains block to be released
- increase `bg_free_blocks_count`, `s_free_blocks_count`

**ext2_truncate(inode)** is used mainly when discarding preallocated blocks

### mOS vfs design

#### File system

```c
typedef struct vfs_file_system_type {
  char *name;
  struct vfs_superblock *mount(struct vfs_filesystem_type *, char *dev_name, char *dir_name);
  int unmount(struct vfs_file_system_type *, char *dev_name, char *dir_name);
} vfs_file_system_type;
```

#### Mount

```c
typedef struct vfs_mount {
  struct dentry *d_root;
  struct dentry *d_mountpoint;
  struct vfs_filesystem_type *fs_type;
  struct vfs_superblock *sb;
  char *dev_name;
} vfs_mount;
```

#### Superblock

```c
typedef struct vfs_superblock {
  struct vfs_mount *mount;
  struct dentry *d_root;
  void *sb_info; // for example -> ext2_superblock
  struct vfs_superblock_operations *s_ops;
} vfs_superblock;

typedef struct vfs_superblock_operations {
  struct vfs_inode *allow_inode(vfs_superblock *);
} vfs_superblock_operations;
```

#### Inode

```c
typedef struct vfs_inode {
  uint32_t ino;
  void *i_info;
  struct vfs_inode_operations *i_ops;
} vfs_inode;

typedef struct vfs_inode_operations {
  struct vfs_dentry *lookup(struct vfs_inode *parent, struct vfs_dentry *child, struct nameidata *);
  int (*create) (struct inode *,struct dentry *, umode_t, bool);
} vfs_inode_operations;
```

#### File

```c
typedef struct vfs_file {
  uint_32t ppos;
  struct vfs_dentry *d_file;
  struct vfs_file_operations *f_ops;
} vfs_file;

typedef struct vfs_file_operations {
  int read(struct vfs_file *f, char *buf, uint32_t ppos, uint32_t size);
  int write(struct vfs_file *f, char *buf, uint32_t ppos, uint32_t size);
  int llseek(struct vfs_file *f, uint32_t ppos);
  int open(struct inode *inode, struct file *f);
} vfs_file_operations;
```

#### Dentry

```c
typedef struct vfs_dentry {
  struct vfs_superblock *sb;
  struct vfs_inode *i_node;
  struct vfs_dentry *d_parent;
  struct list_head subdirs;
  char *name;
  struct vfs_dentry_operations *d_ops;
} vfs_dentry;

typedef struct vfs_dentry_operations {
} vfs_dentry_operations;

typedef struct nameidata {
  struct vfs_dentry *dentry;
  struct vfs_mount *mnt;
} nameidata;
```

#### Process

```c
typedef struct files_struct {
  struct file *fd[256];
} files_struct;

typedef struct fs_struct {
  struct dentry *d_root;
  struct vfs_mount *mnt_root;
} fs_struct;

typedef struct process {
  struct fs_struct *fs;
  struct files_struct *files;
  struct process *parent;
  struct list_head children;
} process;
```

Sections below is demonstrated with ext2

#### Mount

- Before mounting a ext2 device, we have to support ext2 file system. Doing via registering ext2 (store in global file system array)
- invokes `ext2->mount(ext2, "/dev/hba", "/")`
  - create vfs_superblock and ext2_superblock with mountpoint
  - read a ext2 device in order to fill those structures
- initialize `fs_struct` and `files_struct` with information from above and assigns to the current task (define global `current` variable like linux but in a simpler way)

#### Open file

> open(pathname, mode)

- based on current task (`current`)
  - invokes `getname` to read absolute file namepath
  - find a unused slot in `current_process->files->fd`
- break path into components, for each component
  - initialize `nameidata` at root's mountpoint `current_process->fs->d_root/mnt_root` if component is `'/'`
  - search in `nameidata->dentry->subdirs` to find a dentry with `name` = component name -> next
  - if not exist
    - allocate a new dentry based on `nameidata->dentry` and `name`
    - invokes `nameidata->dentry->i_node->lookup(pnode, new_dentry, nd)`
      - looping through inode which belongs pnode to have the `name` above
    - if file doesn't exist and mode is `create`, invoking `nameidata->dentry->i_node->create(pnode, new_dentry, mode, nd)`
      - prefer choosing a new node in the same block group (if parent is not root)
      - search for bitmap of the selected group for the first null bit and set it back 1
      - decrease `bg_free_inodes_count`, `s_free_inodes_count`
      - increase `bg_used_dirs_count`
  - connect the returned dentry into tree (dentry->subdirs)
  - update `nameidata->dentry` with dentry above
  - check if just resolved component refers to a mounted point (go through vfs_mount list) -> update `nameidata->mnt`
- create a file with `nameidata->dentry` and assign it to that unused slot
- return slot number

#### Read file

> read(fd, buf, count)

- `file = current_process->files->fd[fd]`
- invokes `file_ppos` to get file's current position
- `file->fops->read(file, buf, pos, count)`
  - data is divided into pages (each page is ext2 block size)
  - calculate `index = pos >> log blocksize`, `offset = pos % blocksize`
  - for each page, if index is in
    - 0 -> 11 (direct blocks, 1st levels)
      ```c
      ext2_get_block(inode, inode->i_block[1st_block], buf)           // 1st_block = index
      ```
    - 12 -> blocksize/4+11 (2nd level)
      ```c
      ext2_get_block(inode, inode->i_block[2nd_block], buf)           // 2nd_block = 12
      iblock = (uint32_t *)(*buf + 4 * (index - 12))
      ext2_get_block(inode, iblock, buf)
      ```
    - blocksize/4+12 -> (blocksize/4)² + blocksize/4+11 (3rd levels)
      ```c
      ext2_get_block(inode, inode->i_block[3rd_block], buf)           // 3rd_block = 13
      2nd_block = (uint32_t *)(*buf + 4 * (index - 12) / (blocksize/4))
      ext2_get_block(inode, 2nd_block, buf)
      ```
    - (blocksize/4)² + (blocksize/4)+12 -> (blocksize/4)³ + (blocksize/4)² + blocksize/4 + 11 (4th levels)
      ```c
        ext2_get_block(inode, inode->i_block[4th_block], buf)         // 4th_block = 14
        3rd_block = (uint32_t *)(*buf + 4 * (index - 12) / (blocksize/4)²)
        ext2_get_block(inode, 3rd_block, buf)
      ```
  - update `i_atime` to current time

#### Write file

> write(fd, buf, count)

- `file = current_process->files->fd[fd]`
- invokes `file_ppos()` to get file's current position
- `file->fops->write(file, buf, pos, count)`
  - data is divided into pages (each page is ext2 block size)
  - calculate `index = pos >> log blocksize`, `offset = pos % blocksize`
  - for each page
    - invokes `ext2_alloc_block`
      - select a new block (prefered block in the same group)
      - mark corresponding bit in block bitmap to 1
      - decrease `s_free_blocks_count`
    - if index is in
      - 0 -> 11
        ```c
          iblock = ext2_alloc_block(inode)
          inode->i_block[index] = iblock
          ext2_write_inode(inode)
          ext2_write_block(inode, iblock, buf)
        ```
      - 12 -> blocksize/4+11 (2nd level)
        ```c
          if (!inode->i_block[2nd_block]) {
            iblock = ext2_alloc_block(inode)
            inode->i_block[2nd_block] = lblock
            ext2_write_inode(inode)
          }
          ext2_get_block(inode, inode->i_block[2nd_block], buf)           // 2nd_block = 12
          iblock = ext2_alloc_block(inode)
          (uint32_t *)(*buf + 4 * (index - 12)) = iblock
          ext2_write_block(inode, iblock, buf)
        ```
      - blocksize/4+12 -> (blocksize/4)² + blocksize/4+11 (3rd levels)
      - (blocksize/4)² + (blocksize/4)+12 -> (blocksize/4)³ + (blocksize/4)² + blocksize/4 + 11 (4th levels)
