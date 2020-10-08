#include <fs/vfs.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <shared/errno.h>
#include <utils/math.h>
#include <utils/string.h>

#include "tmpfs.h"

static ssize_t tmpfs_read_file(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
	struct vfs_inode *inode = file->f_dentry->d_inode;
	struct vfs_superblock *sb = inode->i_sb;

	count = min_t(size_t, ppos + count, inode->i_size) - ppos;
	uint32_t p = file->f_pos;
	struct page *iter_page;
	char *iter_buf = buf;
	list_for_each_entry(iter_page, &inode->i_data.pages, sibling)
	{
		if (p + sb->s_blocksize < ppos)
			continue;
		if (p >= ppos + count)
			break;

		int32_t pstart = (ppos > p) ? ppos - p : 0;
		uint32_t pend = ((ppos + count) < (p + sb->s_blocksize)) ? (p + sb->s_blocksize - ppos - count) : 0;

		kmap(iter_page);
		memcpy(iter_buf, (char *)iter_page->virtual + pstart, sb->s_blocksize - pstart - pend);
		kunmap(iter_page);
		p += sb->s_blocksize;
		iter_buf += sb->s_blocksize;
	}
	file->f_pos = ppos + count;
	return count;
}

static ssize_t tmpfs_write_file(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
	struct vfs_inode *inode = file->f_dentry->d_inode;
	struct vfs_superblock *sb = inode->i_sb;

	if (ppos + count > inode->i_size)
		tmpfs_setsize(inode, ppos + count);

	uint32_t p = file->f_pos;
	struct page *iter_page;
	char *iter_buf = buf;
	list_for_each_entry(iter_page, &inode->i_data.pages, sibling)
	{
		if (p + sb->s_blocksize < ppos)
			continue;
		if (p >= ppos + count)
			break;

		int32_t pstart = (ppos > p) ? ppos - p : 0;
		uint32_t pend = ((ppos + count) < (p + sb->s_blocksize)) ? (p + sb->s_blocksize - ppos - count) : 0;

		kmap(iter_page);
		memcpy((char *)iter_page->virtual + pstart, iter_buf, sb->s_blocksize - pstart - pend);
		kunmap(iter_page);
		p += sb->s_blocksize;
		iter_buf += sb->s_blocksize;
	}
	file->f_pos = ppos + count;
	return count;
}

static int tmpfs_mmap_file(struct vfs_file *file, struct vm_area_struct *new_vma)
{
	struct vfs_inode *inode = file->f_dentry->d_inode;
	struct vfs_superblock *sb = inode->i_sb;

	struct page *iter_page;
	uint32_t addr = new_vma->vm_start;
	list_for_each_entry(iter_page, &inode->i_data.pages, sibling)
	{
		if (addr >= new_vma->vm_end)
			break;
		vmm_map_address(current_process->pdir, addr, iter_page->frame, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
		addr += sb->s_blocksize;
	}

	return 0;
}

static int tmpfs_release(struct vfs_inode *inode, struct vfs_file *file)
{
	// TODO: MQ 2020-08-22 implement release for `inode->i_data.pages`
	return 0;
}

struct vfs_file_operations tmpfs_file_operations = {
	.llseek = generic_file_llseek,
	.read = tmpfs_read_file,
	.write = tmpfs_write_file,
	.mmap = tmpfs_mmap_file,
	.release = tmpfs_release,
};

struct vfs_file_operations tmpfs_dir_operations = {
	.readdir = generic_memory_readdir,
};
