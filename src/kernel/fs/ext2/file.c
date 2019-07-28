#include <kernel/include/string.h>
#include <kernel/include/errno.h>
#include <kernel/include/common.h>
#include <kernel/fs/vfs.h>
#include <kernel/system/time.h>
#include "ext2.h"

loff_t ext2_llseek_file(vfs_file *file, loff_t ppos)
{
    vfs_inode *inode = file->f_dentry->d_inode;
    ext2_inode *ext2_inode = EXT2_INODE(inode);

    if (ppos > inode->i_size || ppos < 0)
        return -EINVAL;

    file->f_pos = ppos;
    return ppos;
}

ssize_t ext2_read_file(vfs_file *file, char *buf, size_t count, loff_t ppos)
{
    vfs_inode *inode = file->f_dentry->d_inode;
    ext2_inode *ei = EXT2_INODE(inode);
    vfs_superblock *sb = inode->i_sb;

    uint32_t p = (ppos / sb->s_blocksize) * sb->s_blocksize;
    char *iter_buf = buf;
    while (p < ppos + count)
    {
        uint32_t relative_block = p / sb->s_blocksize;
        uint32_t block = 0;
        // FIXME: MQ 2019-07-18 Only support direct blocks
        if (relative_block < 12)
            block = ei->i_block[relative_block];
        char *block_buf = ext2_bread_block(sb, block);
        uint32_t pstart = (ppos > p) ? ppos - p : 0;
        uint32_t pend = ((ppos + count) < (p + sb->s_blocksize)) ? (p + sb->s_blocksize - ppos - count) : 0;
        memcpy(iter_buf, block_buf + pstart, sb->s_blocksize - pstart - pend);
        p += sb->s_blocksize;
        iter_buf += sb->s_blocksize - pstart - pend;
    }
}

ssize_t ext2_write_file(vfs_file *file, char *buf, size_t count, loff_t ppos)
{
    vfs_inode *inode = file->f_dentry->d_inode;
    ext2_inode *ei = EXT2_INODE(inode);
    vfs_superblock *sb = inode->i_sb;

    if (ppos + count > inode->i_size)
    {
        inode->i_size = ppos + count;
        inode->i_blocks = div_ceil(ppos + count, 512);
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
}

vfs_file_operations ext2_file_operations = {
    .llseek = ext2_llseek_file,
    .read = ext2_read_file,
    .write = ext2_write_file,
};

vfs_file_operations ext2_dir_operations = {};