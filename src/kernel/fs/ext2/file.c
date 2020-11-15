#include <fs/buffer.h>
#include <fs/vfs.h>
#include <include/errno.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <system/time.h>
#include <utils/math.h>
#include <utils/printf.h>
#include <utils/string.h>

#include "ext2.h"

static void ext2_read_nth_block(struct vfs_superblock *sb, uint32_t block, char **iter_buf, loff_t ppos, uint32_t *p, size_t count, int level)
{
	assert(level <= 3);

	if (level > 0)
	{
		uint32_t *block_buf = (uint32_t *)ext2_bread_block(sb, block);
		for (uint32_t i = 0, times = sb->s_blocksize / 4; *p < ppos + count && i < times; ++i)
			ext2_read_nth_block(sb, block_buf[i], iter_buf, ppos, p, count, level - 1);
	}
	else
	{
		char *block_buf = ext2_bread_block(sb, block);
		int32_t pstart = (ppos > *p) ? ppos - *p : 0;
		uint32_t pend = ((ppos + count) < (*p + sb->s_blocksize)) ? (*p + sb->s_blocksize - ppos - count) : 0;
		memcpy(*iter_buf, block_buf + pstart, sb->s_blocksize - pstart - pend);
		ext2_bwrite_block(sb, block, block_buf);
		*p += sb->s_blocksize;
		*iter_buf += sb->s_blocksize - pstart - pend;
	}
}

static ssize_t ext2_read_file(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
	struct vfs_inode *inode = file->f_dentry->d_inode;
	struct ext2_inode *ei = EXT2_INODE(inode);
	struct vfs_superblock *sb = inode->i_sb;

	count = min_t(size_t, ppos + count, ei->i_size) - ppos;
	uint32_t p = (ppos / sb->s_blocksize) * sb->s_blocksize;
	char *iter_buf = buf;
	while (p < ppos + count)
	{
		uint32_t relative_block = p / sb->s_blocksize;
		if (relative_block < EXT2_INO_UPPER_LEVEL0)
			ext2_read_nth_block(sb, ei->i_block[relative_block], &iter_buf, ppos, &p, count, 0);
		else if (relative_block < EXT2_INO_UPPER_LEVEL1)
			ext2_read_nth_block(sb, ei->i_block[12], &iter_buf, ppos, &p, count, 1);
		else if (relative_block < EXT2_INO_UPPER_LEVEL2)
			ext2_read_nth_block(sb, ei->i_block[13], &iter_buf, ppos, &p, count, 2);
		else if (relative_block < EXT2_INO_UPPER_LEVEL3)
			ext2_read_nth_block(sb, ei->i_block[14], &iter_buf, ppos, &p, count, 3);
	}

	file->f_pos = ppos + count;
	return count;
}

static ssize_t ext2_write_file(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
	struct vfs_inode *inode = file->f_dentry->d_inode;
	struct ext2_inode *ei = EXT2_INODE(inode);
	struct vfs_superblock *sb = inode->i_sb;

	if (ppos + count > inode->i_size)
	{
		inode->i_size = ppos + count;
		inode->i_blocks = div_ceil(ppos + count, BYTES_PER_SECTOR);
		sb->s_op->write_inode(inode);
	}

	uint32_t p = (ppos / sb->s_blocksize) * sb->s_blocksize;
	char *iter_buf = buf;
	while (p < ppos + count)
	{
		uint32_t relative_block = p / sb->s_blocksize;
		uint32_t block = 0;
		// FIXME: MQ 2019-07-18 Only support direct blocks
		if (relative_block < 12)
		{
			block = ei->i_block[relative_block];
			if (!block)
			{
				block = ext2_create_block(sb);
				ei->i_block[relative_block] = block;
				inode->i_mtime.tv_sec = get_seconds(NULL);
				sb->s_op->write_inode(inode);
			}
		}
		char *block_buf = ext2_bread_block(sb, block);
		uint32_t pstart = (ppos > p) ? ppos - p : 0;
		uint32_t pend = ((ppos + count) < (p + sb->s_blocksize)) ? (p + sb->s_blocksize - ppos - count) : 0;
		memcpy(block_buf + pstart, iter_buf, sb->s_blocksize - pstart - pend);
		ext2_bwrite_block(sb, block, block_buf);
		p += sb->s_blocksize;
		iter_buf += sb->s_blocksize - pstart - pend;
	}

	file->f_pos = ppos + count;
	return count;
}

static int ext2_readdir(struct vfs_file *file, struct dirent *dirent, unsigned int count)
{
	char *buf = kcalloc(count, sizeof(char));
	count = ext2_read_file(file, buf, count, file->f_pos);

	int entries_size = 0;
	struct dirent *idirent = dirent;
	for (char *ibuf = buf; ibuf - buf < count;)
	{
		struct ext2_dir_entry *entry = (struct ext2_dir_entry *)ibuf;
		idirent->d_ino = entry->ino;
		idirent->d_off = 0;
		idirent->d_reclen = sizeof(struct dirent) + entry->name_len + 1;
		idirent->d_type = entry->file_type;
		memcpy(idirent->d_name, entry->name, entry->name_len);

		entries_size += idirent->d_reclen;
		ibuf += entry->rec_len;
		idirent = (struct dirent *)((char *)idirent + idirent->d_reclen);
	}
	return entries_size;
}

static int ext2_mmap_file(struct vfs_file *file, struct vm_area_struct *new_vma)
{
	int length = new_vma->vm_end - new_vma->vm_start;
	kalign_heap(PMM_FRAME_SIZE);
	char *buf = kcalloc(length, sizeof(char));
	loff_t pos = file->f_pos;
	ext2_read_file(file, buf, length, file->f_pos);
	file->f_pos = pos;

	uint32_t addr = new_vma->vm_start;
	for (int i = 0; i < length; i += PMM_FRAME_SIZE)
	{
		vmm_map_address(current_process->pdir, addr, vmm_get_physical_address((uint32_t)buf + i, false), I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
		addr += PMM_FRAME_SIZE;
	}

	return 0;
}

struct vfs_file_operations ext2_file_operations = {
	.llseek = generic_file_llseek,
	.read = ext2_read_file,
	.write = ext2_write_file,
	.mmap = ext2_mmap_file,
};

struct vfs_file_operations ext2_dir_operations = {
	.readdir = ext2_readdir,
};
