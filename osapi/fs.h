/* fs.h */
#pragma once

#include "os.h"
#include "osapi.h"
#include "disk.h"

#define Magic1          (0xdd05)
#define Magic2          (0xaa55)
#define Inodesperblock  (16)
#define PtrPerInode     (8)
#define PtrPerBlock     (256)

typedef int16 ptr;
typedef int8 bootsector[500];
typedef bool bitmap;

enum internal packed {
    TypeNotValid = 0x00,
    TypeFile = 0x01,
    TypeDir = 0x03,
}; 

struct internal packed s_superblock {
    bootsector boot;
    int16 _;
    int16 blocks;
    int16 inodeblocks;
    int16 inodes;
    int16 magic1;
    int16 magic2;
};
typedef struct s_superblock superblock;

struct internal packed s_filesystem {
    int8 drive;
    disk *dd;
    bool *bitmap;
    superblock metadata;
};
typedef struct s_filesystem filesystem;

struct internal packed s_filename {
    int8 name[8];
    int8 ext[3];
};
typedef struct s_filename filename;

struct internal packed s_inode {
    int8 validtype;
    int16 size;
    filename name;
    ptr indirect;
    ptr direct[PtrPerInode];
};
typedef struct s_inode inode;

union internal packed u_fsblock {
    superblock super;
    int8 data[Blocksize];
    ptr pointers[PtrPerBlock];
    inode inodes[Inodesperblock];
};
typedef union u_fsblock fsblock;

internal filesystem *fsformat(disk*,bootsector*,bool);
internal bitmap *mkbitmap(disk*,bool);
block bitmapalloc(bitmap*);
void bitmapfree(bitmap*,block);