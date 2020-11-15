#include <fs/buffer.h>
#include <fs/vfs.h>
#include <include/errno.h>
#include <include/limits.h>
#include <memory/vmm.h>
#include <system/time.h>
#include <utils/debug.h>
#include <utils/math.h>
#include <utils/string.h>

#include "ext2.h"

static int ext2_recursive_block_action(struct vfs_superblock *sb,
									   int level, uint32_t block,
									   const char *name,
									   int (*action)(struct vfs_superblock *, uint32_t, const char *))
{
	assert(level <= 3);
	if (level > 0)
	{
		int ret = -ENOTFOUND;
		uint32_t *block_buf = (uint32_t *)ext2_bread_block(sb, block);
		for (int i = 0, nblocks = sb->s_blocksize / 4; i < nblocks; ++i)
			if ((ret = ext2_recursive_block_action(sb, level - 1, block_buf[i], name, action)) >= 0)
				break;
		return ret;
	}
	else
		return action(sb, block, name);
}

static uint32_t find_unused_block_number(struct vfs_superblock *sb)
{
	struct ext2_superblock *ext2_sb = EXT2_SB(sb);

	uint32_t number_of_groups = div_ceil(ext2_sb->s_blocks_count, ext2_sb->s_blocks_per_group);
	for (uint32_t group = 0; group < number_of_groups; group += 1)
	{
		struct ext2_group_desc *gdp = ext2_get_group_desc(sb, group);
		unsigned char *block_bitmap = (unsigned char *)ext2_bread_block(sb, gdp->bg_block_bitmap);

		for (uint32_t i = 0; i < sb->s_blocksize; ++i)
			if (block_bitmap[i] != 0xff)
				for (int j = 0; j < 8; ++j)
					if (!(block_bitmap[i] & (1 << j)))
						return group * ext2_sb->s_blocks_per_group + i * 8 + j + ext2_sb->s_first_data_block;
	}
	return -ENOSPC;
}

uint32_t ext2_create_block(struct vfs_superblock *sb)
{
	struct ext2_superblock *ext2_sb = EXT2_SB(sb);
	uint32_t block = find_unused_block_number(sb);

	// superblock
	ext2_sb->s_free_blocks_count -= 1;
	sb->s_op->write_super(sb);

	// group
	struct ext2_group_desc *gdp = ext2_get_group_desc(sb, get_group_from_block(ext2_sb, block));
	gdp->bg_free_blocks_count -= 1;
	ext2_write_group_desc(sb, gdp);

	// block bitmap
	char *bitmap_buf = ext2_bread_block(sb, gdp->bg_block_bitmap);
	uint32_t relative_block = get_relative_block_in_group(ext2_sb, block);
	bitmap_buf[relative_block / 8] |= 1 << (relative_block % 8);
	ext2_bwrite_block(sb, gdp->bg_block_bitmap, bitmap_buf);

	// clear block data
	char *data_buf = kcalloc(sb->s_blocksize, sizeof(char));
	ext2_bwrite_block(sb, block, data_buf);

	return block;
}

static uint32_t find_unused_inode_number(struct vfs_superblock *sb)
{
	struct ext2_superblock *ext2_sb = EXT2_SB(sb);

	uint32_t number_of_groups = div_ceil(ext2_sb->s_blocks_count, ext2_sb->s_blocks_per_group);
	for (uint32_t group = 0; group < number_of_groups; group += 1)
	{
		struct ext2_group_desc *gdp = ext2_get_group_desc(sb, group);
		unsigned char *inode_bitmap = (unsigned char *)ext2_bread_block(sb, gdp->bg_inode_bitmap);

		for (uint32_t i = 0; i < sb->s_blocksize; ++i)
			if (inode_bitmap[i] != 0xff)
				for (uint8_t j = 0; j < 8; ++j)
					if (!(inode_bitmap[i] & (1 << j)))
						return group * ext2_sb->s_inodes_per_group + i * 8 + j + EXT2_STARTING_INO;
	}
	return -ENOSPC;
}

static struct vfs_inode *ext2_create_inode(struct vfs_inode *dir, char *filename, mode_t mode)
{
	struct ext2_superblock *ext2_sb = EXT2_SB(dir->i_sb);
	uint32_t ino = find_unused_inode_number(dir->i_sb);
	struct ext2_group_desc *gdp = ext2_get_group_desc(dir->i_sb, get_group_from_inode(ext2_sb, ino));

	// superblock
	ext2_sb->s_free_inodes_count -= 1;
	dir->i_sb->s_op->write_super(dir->i_sb);

	// group descriptor
	gdp->bg_free_inodes_count -= 1;
	if (S_ISDIR(mode))
		gdp->bg_used_dirs_count += 1;
	ext2_write_group_desc(dir->i_sb, gdp);

	// inode bitmap
	char *inode_bitmap_buf = ext2_bread_block(dir->i_sb, gdp->bg_inode_bitmap);
	uint32_t relative_inode = get_relative_inode_in_group(ext2_sb, ino);
	inode_bitmap_buf[relative_inode / 8] |= 1 << (relative_inode % 8);
	ext2_bwrite_block(dir->i_sb, gdp->bg_inode_bitmap, inode_bitmap_buf);

	// inode table
	struct ext2_inode *ei_new = kcalloc(1, sizeof(struct ext2_inode));
	ei_new->i_links_count = 1;
	struct vfs_inode *inode = dir->i_sb->s_op->alloc_inode(dir->i_sb);
	inode->i_ino = ino;
	inode->i_mode = mode;
	inode->i_size = 0;
	inode->i_fs_info = ei_new;
	inode->i_sb = dir->i_sb;
	inode->i_atime.tv_sec = get_seconds(NULL);
	inode->i_ctime.tv_sec = get_seconds(NULL);
	inode->i_mtime.tv_sec = get_seconds(NULL);
	inode->i_flags = 0;
	inode->i_blocks = 0;

	if (S_ISREG(mode))
	{
		inode->i_op = &ext2_file_inode_operations;
		inode->i_fop = &ext2_file_operations;
	}
	else if (S_ISDIR(mode))
	{
		inode->i_op = &ext2_dir_inode_operations;
		inode->i_fop = &ext2_dir_operations;

		struct ext2_inode *ei = EXT2_INODE(inode);
		uint32_t block = ext2_create_block(inode->i_sb);
		ei->i_block[0] = block;
		inode->i_blocks += 2;
		inode->i_size += 1024;
		ext2_write_inode(inode);

		char *block_buf = ext2_bread_block(inode->i_sb, block);

		struct ext2_dir_entry *c_entry = (struct ext2_dir_entry *)block_buf;
		c_entry->ino = inode->i_ino;
		memcpy(c_entry->name, ".", 1);
		c_entry->name_len = 1;
		c_entry->rec_len = EXT2_DIR_REC_LEN(1);
		c_entry->file_type = 2;

		struct ext2_dir_entry *p_entry = (struct ext2_dir_entry *)(block_buf + c_entry->rec_len);
		p_entry->ino = dir->i_ino;
		memcpy(p_entry->name, "..", 2);
		p_entry->name_len = 2;
		p_entry->rec_len = 1024 - c_entry->rec_len;
		p_entry->file_type = 2;

		ext2_bwrite_block(inode->i_sb, block, block_buf);
	}
	dir->i_sb->s_op->write_inode(inode);

	// FIXME: MQ 2019-07-16 Only support direct blocks
	for (int i = 0; i < 11; ++i)
	{
		struct ext2_inode *ei = EXT2_INODE(dir);
		int block = ei->i_block[i];
		if (!block)
		{
			block = ext2_create_block(dir->i_sb);
			ei->i_block[i] = block;
			dir->i_blocks += 2;
			dir->i_size += 1024;
			ext2_write_inode(dir);
		}
		char *block_buf = ext2_bread_block(dir->i_sb, block);

		uint32_t size = 0, new_rec_len = 0;
		struct ext2_dir_entry *entry = (struct ext2_dir_entry *)block_buf;
		while (size < dir->i_sb->s_blocksize && (char *)entry < block_buf + dir->i_sb->s_blocksize)
		{
			if (!entry->ino)
			{
				entry->ino = inode->i_ino;
				if (S_ISREG(inode->i_mode))
				{
					entry->file_type = 1;
				}
				else if (S_ISDIR(inode->i_mode))
				{
					entry->file_type = 2;
				}
				entry->name_len = strlen(filename);
				memcpy(entry->name, filename, entry->name_len);
				entry->rec_len = new_rec_len;

				ext2_bwrite_block(dir->i_sb, block, block_buf);
				return inode;
			}
			if (EXT2_DIR_REC_LEN(strlen(filename)) + EXT2_DIR_REC_LEN(entry->name_len) < entry->rec_len)
			{
				size += EXT2_DIR_REC_LEN(entry->name_len);
				new_rec_len = entry->rec_len - EXT2_DIR_REC_LEN(entry->name_len);
				entry->rec_len = EXT2_DIR_REC_LEN(entry->name_len);
				entry = (struct ext2_dir_entry *)((char *)entry + EXT2_DIR_REC_LEN(entry->name_len));
			}
			else
			{
				size += entry->rec_len;
				new_rec_len = 1024 - size;
				entry = (struct ext2_dir_entry *)((char *)entry + entry->rec_len);
			}
		}
	}
	return NULL;
}

int ext2_find_ino(struct vfs_superblock *sb, uint32_t block, const char *name)
{
	char *block_buf = ext2_bread_block(sb, block);

	char tmpname[NAME_MAX];
	uint32_t size = 0;
	struct ext2_dir_entry *entry = (struct ext2_dir_entry *)block_buf;
	while (size < sb->s_blocksize && entry->ino != 0)
	{
		memcpy(tmpname, entry->name, entry->name_len);
		tmpname[entry->name_len] = 0;

		if (strcmp(tmpname, name) == 0)
			return entry->ino;

		entry = (struct ext2_dir_entry *)((char *)entry + entry->rec_len);
		size = size + entry->rec_len;
	}
	return -ENOENT;
}

static struct vfs_inode *ext2_lookup_inode(struct vfs_inode *dir, char *name)
{
	struct ext2_inode *ei = EXT2_INODE(dir);
	struct vfs_superblock *sb = dir->i_sb;

	for (int i = 0, ino = 0; i < ei->i_blocks; ++i)
	{
		if (i < EXT2_INO_UPPER_LEVEL0)
		{
			ino = ext2_recursive_block_action(sb, 0, ei->i_block[i], name, ext2_find_ino);
		}
		else
		{
		}
		if ((i < EXT2_INO_UPPER_LEVEL0 && (ino = ext2_recursive_block_action(sb, 0, ei->i_block[i], name, ext2_find_ino)) > 0) ||
			((EXT2_INO_UPPER_LEVEL0 <= i && i < EXT2_INO_UPPER_LEVEL1) && (ino = ext2_recursive_block_action(sb, 1, ei->i_block[12], name, ext2_find_ino)) > 0) ||
			((EXT2_INO_UPPER_LEVEL1 <= i && i < EXT2_INO_UPPER_LEVEL2) && (ino = ext2_recursive_block_action(sb, 2, ei->i_block[13], name, ext2_find_ino)) > 0) ||
			((EXT2_INO_UPPER_LEVEL2 <= i && i < EXT2_INO_UPPER_LEVEL3) && (ino = ext2_recursive_block_action(sb, 3, ei->i_block[14], name, ext2_find_ino)) > 0))
		{
			struct vfs_inode *inode = dir->i_sb->s_op->alloc_inode(dir->i_sb);
			inode->i_ino = ino;
			ext2_read_inode(inode);
			return inode;
		}
	}
	return NULL;
}

static void ext2_truncate_inode(struct vfs_inode *i)
{
}

static int ext2_mknod(struct vfs_inode *dir, struct vfs_dentry *dentry, int mode, dev_t dev)
{
	struct vfs_inode *inode = ext2_lookup_inode(dir, dentry->d_name);
	if (inode == NULL)
		inode = ext2_create_inode(dir, dentry->d_name, mode);
	inode->i_rdev = dev;
	init_special_inode(inode, mode, dev);
	ext2_write_inode(inode);

	dentry->d_inode = inode;
	return 0;
}

static int ext2_delete_entry(struct vfs_superblock *sb, uint32_t block, const char *name)
{
	char *block_buf = ext2_bread_block(sb, block);

	char tmpname[NAME_MAX];
	struct ext2_dir_entry *prev = NULL;
	struct ext2_dir_entry *entry = (struct ext2_dir_entry *)block_buf;
	for (uint32_t size = 0; size < sb->s_blocksize && entry->ino != 0; size += entry->rec_len)
	{
		memcpy(tmpname, entry->name, entry->name_len);
		tmpname[entry->name_len] = 0;

		if (strcmp(tmpname, name) == 0)
		{
			int ino = entry->ino;
			entry->ino = 0;

			if (prev)
				prev->rec_len += entry->rec_len;

			return ino;
		}

		prev = entry;
		entry = (struct ext2_dir_entry *)((char *)entry + entry->rec_len);
	}
	return -ENOENT;
}

int ext2_unlink(struct vfs_inode *dir, struct vfs_dentry *dentry)
{
	struct ext2_inode *ei = EXT2_INODE(dir);
	struct vfs_superblock *sb = dir->i_sb;

	for (int i = 0, ino = 0; i < ei->i_blocks; ++i)
	{
		if ((i < EXT2_INO_UPPER_LEVEL0 && (ino = ext2_recursive_block_action(sb, 0, ei->i_block[i], dentry->d_name, ext2_delete_entry)) > 0) ||
			((EXT2_INO_UPPER_LEVEL0 <= i && i < EXT2_INO_UPPER_LEVEL1) && (ino = ext2_recursive_block_action(sb, 1, ei->i_block[12], dentry->d_name, ext2_delete_entry)) > 0) ||
			((EXT2_INO_UPPER_LEVEL1 <= i && i < EXT2_INO_UPPER_LEVEL2) && (ino = ext2_recursive_block_action(sb, 2, ei->i_block[13], dentry->d_name, ext2_delete_entry)) > 0) ||
			((EXT2_INO_UPPER_LEVEL2 <= i && i < EXT2_INO_UPPER_LEVEL3) && (ino = ext2_recursive_block_action(sb, 3, ei->i_block[14], dentry->d_name, ext2_delete_entry)) > 0))
		{
			struct vfs_inode *inode = dir->i_sb->s_op->alloc_inode(dir->i_sb);
			inode->i_ino = ino;
			ext2_read_inode(inode);

			inode->i_nlink -= 1;
			ext2_write_inode(inode);
			// TODO: If i_nlink == 0, should we delete ext2 inode?

			break;
		}
	}
	return 0;
}

struct vfs_inode_operations ext2_file_inode_operations = {
	.truncate = ext2_truncate_inode,
};

struct vfs_inode_operations ext2_dir_inode_operations = {
	.create = ext2_create_inode,
	.lookup = ext2_lookup_inode,
	.mknod = ext2_mknod,
	.unlink = ext2_unlink,

};

struct vfs_inode_operations ext2_special_inode_operations = {};
