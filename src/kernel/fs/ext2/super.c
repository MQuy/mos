#include <devices/ata.h>
#include <fs/buffer.h>
#include <fs/vfs.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <shared/errno.h>
#include <utils/math.h>
#include <utils/string.h>

#include "ext2.h"

struct ext2_group_desc *ext2_get_group_desc(struct vfs_superblock *sb, uint32_t group)
{
	struct ext2_group_desc *gdp = kcalloc(1, sizeof(struct ext2_group_desc));
	struct ext2_superblock *ext2_sb = EXT2_SB(sb);
	uint32_t block = ext2_sb->s_first_data_block + 1 + group / EXT2_GROUPS_PER_BLOCK(ext2_sb);
	char *group_block_buf = ext2_bread_block(sb, block);
	uint32_t offset = group % EXT2_GROUPS_PER_BLOCK(ext2_sb) * sizeof(struct ext2_group_desc);
	memcpy(gdp, group_block_buf + offset, sizeof(struct ext2_group_desc));
	return gdp;
}

void ext2_write_group_desc(struct vfs_superblock *sb, struct ext2_group_desc *gdp)
{
	struct ext2_superblock *ext2_sb = EXT2_SB(sb);
	uint32_t group = get_group_from_block(ext2_sb, gdp->bg_block_bitmap);
	uint32_t block = ext2_sb->s_first_data_block + 1 + group / EXT2_GROUPS_PER_BLOCK(ext2_sb);
	uint32_t offset = group % EXT2_GROUPS_PER_BLOCK(ext2_sb) * sizeof(struct ext2_group_desc);
	char *group_block_buf = ext2_bread_block(sb, block);
	memcpy(group_block_buf + offset, gdp, sizeof(struct ext2_group_desc));
	ext2_bwrite_block(sb, block, group_block_buf);
}

static struct ext2_inode *ext2_get_inode(struct vfs_superblock *sb, ino_t ino)
{
	struct ext2_superblock *ext2_sb = EXT2_SB(sb);
	uint32_t group = get_group_from_inode(ext2_sb, ino);
	struct ext2_group_desc *gdp = ext2_get_group_desc(sb, group);
	uint32_t block = gdp->bg_inode_table + get_relative_inode_in_group(ext2_sb, ino) / EXT2_INODES_PER_BLOCK(ext2_sb);
	uint32_t offset = (get_relative_inode_in_group(ext2_sb, ino) % EXT2_INODES_PER_BLOCK(ext2_sb)) * sizeof(struct ext2_inode);
	char *table_buf = ext2_bread_block(sb, block);

	return (struct ext2_inode *)(table_buf + offset);
}

struct vfs_inode *ext2_alloc_inode(struct vfs_superblock *sb)
{
	struct vfs_inode *i = init_inode();
	i->i_sb = sb;
	atomic_set(&i->i_count, 1);

	return i;
}

void ext2_read_inode(struct vfs_inode *i)
{
	struct ext2_inode *raw_node = ext2_get_inode(i->i_sb, i->i_ino);

	i->i_mode = raw_node->i_mode;
	i->i_gid = raw_node->i_gid;
	i->i_uid = raw_node->i_uid;

	i->i_nlink = raw_node->i_links_count;
	i->i_size = raw_node->i_size;
	i->i_atime.tv_sec = raw_node->i_atime;
	i->i_ctime.tv_sec = raw_node->i_ctime;
	i->i_mtime.tv_sec = raw_node->i_mtime;
	i->i_atime.tv_nsec = i->i_ctime.tv_nsec = i->i_mtime.tv_nsec = 0;

	i->i_blksize = PMM_FRAME_SIZE; /* This is the optimal IO size (for stat), not the fs block size */
	i->i_blocks = raw_node->i_blocks;
	i->i_flags = raw_node->i_flags;
	i->i_fs_info = raw_node;

	if (S_ISREG(i->i_mode))
	{
		i->i_op = &ext2_file_inode_operations;
		i->i_fop = &ext2_file_operations;
	}
	else if (S_ISDIR(i->i_mode))
	{
		i->i_op = &ext2_dir_inode_operations;
		i->i_fop = &ext2_dir_operations;
	}
	else
	{
		i->i_op = &ext2_special_inode_operations;
		init_special_inode(i, i->i_mode, raw_node->i_block[0]);
	}
}

void ext2_write_inode(struct vfs_inode *i)
{
	struct ext2_superblock *ext2_sb = EXT2_SB(i->i_sb);
	struct ext2_inode *ei = EXT2_INODE(i);

	ei->i_mode = i->i_mode;
	ei->i_gid = i->i_gid;
	ei->i_uid = i->i_uid;

	ei->i_size = i->i_size;
	ei->i_atime = i->i_atime.tv_sec;
	ei->i_ctime = i->i_ctime.tv_sec;
	ei->i_mtime = i->i_mtime.tv_sec;

	ei->i_blocks = i->i_blocks;
	ei->i_flags = i->i_flags;

	if (S_ISCHR(i->i_mode))
	{
		ei->i_block[0] = i->i_rdev;
	}

	uint32_t group = get_group_from_inode(ext2_sb, i->i_ino);
	struct ext2_group_desc *gdp = ext2_get_group_desc(i->i_sb, group);
	uint32_t block = gdp->bg_inode_table + get_relative_inode_in_group(ext2_sb, i->i_ino) / EXT2_INODES_PER_BLOCK(ext2_sb);
	uint32_t offset = (get_relative_inode_in_group(ext2_sb, i->i_ino) % EXT2_INODES_PER_BLOCK(ext2_sb)) * sizeof(struct ext2_inode);
	char *buf = ext2_bread_block(i->i_sb, block);

	memcpy(buf + offset, ei, sizeof(struct ext2_inode));
	ext2_bwrite_block(i->i_sb, block, buf);
}

static void ext2_write_super(struct vfs_superblock *sb)
{
	struct ext2_superblock *ext2_sb = EXT2_SB(sb);
	ext2_bwrite_block(sb, ext2_sb->s_first_data_block, (char *)ext2_sb);
}

struct vfs_super_operations ext2_super_operations = {
	.alloc_inode = ext2_alloc_inode,
	.read_inode = ext2_read_inode,
	.write_inode = ext2_write_inode,
	.write_super = ext2_write_super,
};

static int ext2_fill_super(struct vfs_superblock *sb)
{
	struct ext2_superblock *ext2_sb = (struct ext2_superblock *)kcalloc(1, sizeof(struct ext2_superblock));
	char *buf = ext2_bread_block(sb, 1);
	memcpy(ext2_sb, (struct ext2_superblock *)buf, sb->s_blocksize);

	if (ext2_sb->s_magic != EXT2_SUPER_MAGIC)
		return -EINVAL;

	sb->s_fs_info = ext2_sb;
	sb->s_op = &ext2_super_operations;
	sb->s_blocksize = EXT2_BLOCK_SIZE(ext2_sb);
	sb->s_blocksize_bits = ext2_sb->s_log_block_size;
	sb->s_magic = EXT2_SUPER_MAGIC;
	return 0;
}

static struct vfs_mount *ext2_mount(struct vfs_file_system_type *fs_type,
									char *dev_name, char *dir_name)
{
	struct vfs_superblock *sb = (struct vfs_superblock *)kcalloc(1, sizeof(struct vfs_superblock));
	sb->s_blocksize = EXT2_MIN_BLOCK_SIZE;
	sb->mnt_devname = dev_name;
	sb->s_type = fs_type;
	ext2_fill_super(sb);

	struct vfs_inode *i_root = ext2_alloc_inode(sb);
	i_root->i_ino = EXT2_ROOT_INO;
	ext2_read_inode(i_root);

	struct vfs_dentry *d_root = alloc_dentry(NULL, dir_name);
	d_root->d_inode = i_root;
	d_root->d_sb = sb;

	sb->s_root = d_root;

	struct vfs_mount *mnt = kcalloc(1, sizeof(struct vfs_mount));
	mnt->mnt_sb = sb;
	mnt->mnt_mountpoint = mnt->mnt_root = sb->s_root;
	mnt->mnt_devname = dev_name;

	return mnt;
}

struct vfs_file_system_type ext2_fs_type = {
	.name = "ext2",
	.mount = ext2_mount,
};

void init_ext2_fs()
{
	register_filesystem(&ext2_fs_type);
}

void exit_ext2_fs()
{
	unregister_filesystem(&ext2_fs_type);
}

char *ext2_bread_block(struct vfs_superblock *sb, uint32_t block)
{
	return ext2_bread(sb, block, sb->s_blocksize);
}

char *ext2_bread(struct vfs_superblock *sb, uint32_t block, uint32_t size)
{
	return bread(sb->mnt_devname, block * (sb->s_blocksize / 512), size);
}

void ext2_bwrite_block(struct vfs_superblock *sb, uint32_t block, char *buf)
{
	return ext2_bwrite(sb, block, buf, sb->s_blocksize);
}

void ext2_bwrite(struct vfs_superblock *sb, uint32_t block, char *buf, uint32_t size)
{
	return bwrite(sb->mnt_devname, block * (sb->s_blocksize / 512), buf, size);
}
