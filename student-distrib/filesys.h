#ifndef FILESYS_H
#define FILESYS_H

#include "types.h"
#include "lib.h"

#define BLOCK_SIZE          4096
#define DENTRIES_NUM        63
#define NAME_LENGTH         32
#define DBLOCKS_IN_INODE    1023
#define DENTRY_PADDING      24
#define BOOT_BLK_PADDING    52

// we might have to remember the offset, do this later, if no one found out then we dont do it


/* check MP3 Appendix A for detailed explanation of these */
typedef struct dentry_t {
    uint8_t  name[NAME_LENGTH];
    uint32_t type;
    uint32_t inode_num;
    uint8_t  reserved[DENTRY_PADDING];
} __attribute__((packed)) dentry_t;

typedef struct boot_block {
    uint32_t num_dentries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t  reserved[BOOT_BLK_PADDING];
    dentry_t dentries[DENTRIES_NUM];
} __attribute__((packed)) boot_block;

typedef struct inode_t {
    uint32_t length;
    uint32_t data_blocks[DBLOCKS_IN_INODE];
} __attribute__((packed)) inode_t;

typedef struct data_block {
    uint8_t data[BLOCK_SIZE];
} __attribute__((packed)) data_block;

typedef struct filesys {
    boot_block* boot_block_pointer;
    inode_t*    inode_pointer;
    data_block* data_block_pointer;
} __attribute__ ((packed)) filesys;



/* initializes the file system */
extern void file_system_init(void* start_addr);

/* file operations */
extern int32_t open_file(const uint8_t* filename, int32_t fd);
extern int32_t close_file(int32_t fd);
extern int32_t read_file(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);

/* modified functions for execute syscall */
extern int32_t read_file_execute(void* buf, int32_t nbytes);
extern int32_t open_file_execute(const uint8_t* filename);
/* directory operations */
extern int32_t open_dir(const uint8_t* filename, int32_t fd);
extern int32_t close_dir(int32_t fd);
extern int32_t read_dir(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes);

/* dentry/data operations */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif

