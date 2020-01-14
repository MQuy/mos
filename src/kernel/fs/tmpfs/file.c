#include <include/errno.h>
#include <include/ctype.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include "tmpfs.h"

extern process *current_process;

loff_t tmpfs_llseek_file(vfs_file *file, loff_t ppos)
{
  vfs_inode *inode = file->f_dentry->d_inode;

  if (ppos > inode->i_size || ppos < 0)
    return -EINVAL;

  file->f_pos = ppos;
  return ppos;
}

ssize_t tmpfs_read_file(vfs_file *file, char *buf, size_t count, loff_t ppos)
{
  vfs_inode *inode = file->f_dentry->d_inode;
  vfs_superblock *sb = inode->i_sb;

  if (ppos + count > inode->i_size)
    return -1;

  uint32_t p = 0;
  page *iter_page;
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
    memcpy(iter_buf, iter_page->virtual + pstart, sb->s_blocksize - pstart - pend);
    kunmap(iter_page);
    p += sb->s_blocksize;
    iter_buf += sb->s_blocksize;
  }
  file->f_pos = ppos + count;
  return count;
}

ssize_t tmpfs_write_file(vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
  vfs_inode *inode = file->f_dentry->d_inode;
  vfs_superblock *sb = inode->i_sb;

  if (ppos + count > inode->i_size)
    tmpfs_setsize(inode, ppos + count);

  uint32_t p = 0;
  page *iter_page;
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
    memcpy(iter_page->virtual + pstart, iter_buf, sb->s_blocksize - pstart - pend);
    kunmap(iter_page);
    p += sb->s_blocksize;
    iter_buf += sb->s_blocksize;
  }
  file->f_pos = ppos + count;
  return count;
}

int tmpfs_mmap_file(vfs_file *file, vm_area_struct *new_vma)
{
  vfs_inode *inode = file->f_dentry->d_inode;
  vfs_superblock *sb = inode->i_sb;
  vm_area_struct *vma = inode->i_data.i_mmap;

  page *iter_page;
  uint32_t addr = new_vma->vm_start;
  list_for_each_entry(iter_page, &inode->i_data.pages, sibling)
  {
    if (addr >= new_vma->vm_end)
      break;
    vmm_map_address(current_process->pdir, addr, iter_page->frame, I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER);
    addr += sb->s_blocksize;
  }

  if (!vma)
    inode->i_data.i_mmap = new_vma;
  else if (vma->vm_start == new_vma->vm_start)
  {
    if (vma->vm_end > new_vma->vm_end)
      for (uint32_t addr = new_vma->vm_end; addr < vma->vm_end; addr += PMM_FRAME_SIZE)
        vmm_unmap_address(current_process->pdir, addr);
  }
  else
    do_munmap(current_process->pdir, vma->vm_start, vma->vm_end - vma->vm_start);
  return 0;
}

vfs_file_operations tmpfs_file_operations = {
    .llseek = tmpfs_llseek_file,
    .read = tmpfs_read_file,
    .write = tmpfs_write_file,
    .mmap = tmpfs_mmap_file,
};

vfs_file_operations tmpfs_dir_operations = {};