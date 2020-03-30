#ifndef FS_EXT2_H
#define FS_EXT2_H

#include <stdint.h>
#include <kernel/fs/vfs.h>

/*
 * Special struct vfs_inode numbers
 */
#define EXT2_BAD_INO 1         /* Bad blocks vfs_inode */
#define EXT2_ROOT_INO 2        /* Root vfs_inode */
#define EXT2_BOOT_LOADER_INO 5 /* Boot loader vfs_inode */
#define EXT2_UNDEL_DIR_INO 6   /* Undelete directory vfs_inode */
#define EXT2_STARTING_INO 1

#define EXT2_SUPER_MAGIC 0xEF53

#define EXT2_DIR_PAD 4
#define EXT2_DIR_ROUND (EXT2_DIR_PAD - 1)
#define EXT2_DIR_REC_LEN(name_len) (((name_len) + 8 + EXT2_DIR_ROUND) & \
                                    ~EXT2_DIR_ROUND)

struct ext2_superblock
{
  uint32_t s_inodes_count;      /* Inodes count */
  uint32_t s_blocks_count;      /* Blocks count */
  uint32_t s_r_blocks_count;    /* Reserved blocks count */
  uint32_t s_free_blocks_count; /* Free blocks count */
  uint32_t s_free_inodes_count; /* Free inodes count */
  uint32_t s_first_data_block;  /* First Data Block */
  uint32_t s_log_block_size;    /* Block size */
  uint32_t s_log_frag_size;     /* Fragment size */
  uint32_t s_blocks_per_group;  /* # Blocks per group */
  uint32_t s_frags_per_group;   /* # Fragments per group */
  uint32_t s_inodes_per_group;  /* # Inodes per group */
  uint32_t s_mtime;             /* Mount time */
  uint32_t s_wtime;             /* Write time */
  uint16_t s_mnt_count;         /* Mount count */
  uint16_t s_max_mnt_count;     /* Maximal mount count */
  uint16_t s_magic;             /* Magic signature */
  uint16_t s_state;             /* File system state */
  uint16_t s_errors;            /* Behaviour when detecting errors */
  uint16_t s_minor_rev_level;   /* minor revision level */
  uint32_t s_lastcheck;         /* time of last check */
  uint32_t s_checkinterval;     /* max. time between checks */
  uint32_t s_creator_os;        /* OS */
  uint32_t s_rev_level;         /* Revision level */
  uint16_t s_def_resuid;        /* Default uid for reserved blocks */
  uint16_t s_def_resgid;        /* Default gid for reserved blocks */

  uint32_t s_first_ino;              /* First non-reserved inode */
  uint16_t s_inode_size;             /* size of inode structure */
  uint16_t s_block_group_nr;         /* block group # of this superblock */
  uint32_t s_feature_compat;         /* compatible feature set */
  uint32_t s_feature_incompat;       /* incompatible feature set */
  uint32_t s_feature_ro_compat;      /* readonly-compatible feature set */
  uint8_t s_uuid[16];                /* 128-bit uuid for volume */
  char s_volume_name[16];            /* volume name */
  char s_last_mounted[64];           /* directory where last mounted */
  uint32_t s_algorithm_usage_bitmap; /* For compression */

  uint8_t s_prealloc_blocks;     /* Nr of blocks to try to preallocate*/
  uint8_t s_prealloc_dir_blocks; /* Nr to preallocate for dirs */
  uint16_t s_padding1;

  uint8_t s_journal_uuid[16]; /* uuid of journal superblock */
  uint32_t s_journal_inum;    /* inode number of journal file */
  uint32_t s_journal_dev;     /* device number of journal file */
  uint32_t s_last_orphan;     /* start of list of inodes to delete */
  uint32_t s_hash_seed[4];    /* HTREE hash seed */
  uint8_t s_def_hash_version; /* Default hash version to use */
  uint8_t s_reserved_char_pad;
  uint16_t s_reserved_word_pad;
  uint32_t s_default_mount_opts;
  uint32_t s_first_meta_bg; /* First metablock block group */
  uint32_t s_reserved[190]; /* Padding to the end of the block */
};

struct ext2_group_desc
{
  uint32_t bg_block_bitmap;
  uint32_t bg_inode_bitmap;
  uint32_t bg_inode_table;
  uint16_t bg_free_blocks_count;
  uint16_t bg_free_inodes_count;
  uint16_t bg_used_dirs_count;
  uint16_t bg_pad;
  uint32_t bg_reserved[3];
};

struct ext2_inode
{
  uint16_t i_mode;        /* File mode */
  uint16_t i_uid;         /* Low 16 bits of Owner Uid */
  uint32_t i_size;        /* Size in bytes */
  uint32_t i_atime;       /* Access time */
  uint32_t i_ctime;       /* Creation time */
  uint32_t i_mtime;       /* Modification time */
  uint32_t i_dtime;       /* Deletion Time */
  uint16_t i_gid;         /* Low 16 bits of Group Id */
  uint16_t i_links_count; /* Links count */
  uint32_t i_blocks;      /* Blocks count */
  uint32_t i_flags;       /* File flags */
  union {
    struct
    {
      uint32_t l_i_reserved1;
    } linux1;
    struct
    {
      uint32_t h_i_translator;
    } hurd1;
    struct
    {
      uint32_t m_i_reserved1;
    } masix1;
  } osd1;                /* OS dependent 1 */
  uint32_t i_block[15];  /* Pointers to blocks */
  uint32_t i_generation; /* File version (for NFS) */
  uint32_t i_file_acl;   /* File ACL */
  uint32_t i_dir_acl;    /* Directory ACL */
  uint32_t i_faddr;      /* Fragment address */
  union {
    struct linux2
    {
      uint8_t l_i_frag;  /* Fragment number */
      uint8_t l_i_fsize; /* Fragment size */
      uint16_t i_pad1;
      uint16_t l_i_uid_high; /* these 2 fields    */
      uint16_t l_i_gid_high; /* were reserved2[0] */
      uint32_t l_i_reserved2;
    } linux2;
    struct
    {
      uint8_t h_i_frag;  /* Fragment number */
      uint8_t h_i_fsize; /* Fragment size */
      uint16_t h_i_mode_high;
      uint16_t h_i_uid_high;
      uint16_t h_i_gid_high;
      uint32_t h_i_author;
    } hurd2;
    struct
    {
      uint8_t m_i_frag;  /* Fragment number */
      uint8_t m_i_fsize; /* Fragment size */
      uint16_t m_pad1;
      uint32_t m_i_reserved2[2];
    } masix2;
  } osd2; /* OS dependent 2 */
};

#define EXT2_NAME_LEN 255

struct ext2_dir_entry
{
  uint32_t ino;
  // NOTE: 2019-07-28 rec_len has to be calculated by EXT2_DIR_REC_LEN
  uint16_t rec_len;
  uint8_t name_len;
  uint8_t file_type;
  char name[EXT2_NAME_LEN];
};

enum
{
  EXT2_FT_UNKNOWN,
  EXT2_FT_REG_FILE,
  EXT2_FT_DIR,
  EXT2_FT_MAX
};

static inline struct ext2_superblock *EXT2_SB(struct vfs_superblock *sb)
{
  return sb->s_fs_info;
}

static inline struct ext2_inode *EXT2_INODE(struct vfs_inode *inode)
{
  return inode->i_fs_info;
}

#define EXT2_MIN_BLOCK_SIZE 1024
#define EXT2_MAX_BLOCK_SIZE 4096

#define EXT2_BLOCK_SIZE(sb) (EXT2_MIN_BLOCK_SIZE << sb->s_log_block_size)
#define EXT2_INODES_PER_BLOCK(sb) (EXT2_BLOCK_SIZE(sb) / sb->s_inode_size)
#define EXT2_GROUPS_PER_BLOCK(sb) (EXT2_BLOCK_SIZE(sb) / sizeof(struct ext2_group_desc))

#define get_group_from_inode(sb, ino) ((ino - EXT2_STARTING_INO) / sb->s_inodes_per_group)
#define get_relative_inode_in_group(sb, ino) ((ino - EXT2_STARTING_INO) % sb->s_inodes_per_group)
#define get_group_from_block(sb, block) ((block - sb->s_first_data_block) / sb->s_blocks_per_group)
#define get_relative_block_in_group(sb, block) ((block - sb->s_first_data_block) % sb->s_blocks_per_group)

// super.c
void init_ext2_fs();
void exit_ext2_fs();
char *ext2_bread_block(struct vfs_superblock *sb, uint32_t iblock);
char *ext2_bread(struct vfs_superblock *sb, uint32_t iblock, uint32_t size);
void ext2_bwrite_block(struct vfs_superblock *sb, uint32_t iblock, char *buf);
void ext2_bwrite(struct vfs_superblock *sb, uint32_t iblock, char *buf, uint32_t size);
struct vfs_inode *ext2_alloc_inode(struct vfs_superblock *sb);
void ext2_read_inode(struct vfs_inode *);
void ext2_write_inode(struct vfs_inode *);
struct ext2_group_desc *ext2_get_group_desc(struct vfs_superblock *sb, uint32_t block_group);
void ext2_write_group_desc(struct vfs_superblock *sb, struct ext2_group_desc *gdp);

extern struct vfs_super_operations ext2_super_operations;

// vfs_inode.c
uint32_t ext2_create_block(struct vfs_superblock *sb);

extern struct vfs_inode_operations ext2_dir_inode_operations;
extern struct vfs_inode_operations ext2_file_inode_operations;
extern struct vfs_inode_operations ext2_special_inode_operations;

// file.c
extern struct vfs_file_operations ext2_file_operations;
extern struct vfs_file_operations ext2_dir_operations;

extern struct vfs_file_operations def_chr_fops;

#endif