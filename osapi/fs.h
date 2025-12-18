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
#define ValidChars      ($1 "abcdefghijklmnopqrstuvwxyz0123456789_-")
#define ValidPathChars  ($1 "abcdefghijklmnopqrstuvwxyz0123456789_-/:.")
#define DirDepth        (16) 
#define MaxFilesPerDir  (PtrPerInode+PtrPerBlock)
#define MaxOpenFiles    (256)
#define Totalblocks     (65535)   

typedef int16 ptr;   
typedef int8 bootsector[500];
typedef bool bitmap;
// typedef int8 path;   


enum internal packed  e_type {
    TypeNotValid = 0x00,  
    TypeFile = 0x01,
    TypeDir = 0x03,
}; 
typedef enum e_type type;  

struct internal packed s_superblock {
    bootsector boot;   /* 500 */
    int16 _;           /* 502 */
    int16 blocks;      /* 504 */
    int16 inodeblocks; /* 506 */
    int16 inodes;      /* 508 */
    int16 magic1;      /* 510 */
    int16 magic2;      /* 512 */
};
typedef struct s_superblock superblock;

struct internal packed s_filesystem {
    int8 drive:2;
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

struct public packed s_fileinfo {
    ptr idx;
    int16 size;
};
typedef struct s_fileinfo fileinfo;

struct  internal packed s_path {
   filesystem *fs;
   int16 drive;
   filename target;
   int8 dirpath[DirDepth+1][9];
};
typedef struct s_path path;

struct public packed s_fileentry {
    ptr inode;
    filename name;
    int16 size;
    type filetype;
};
typedef struct s_fileentry fileentry;

struct public packed s_directory {
    int16 drive;
    filesystem *fs;
    ptr inode;
    filename *dirname;
    int16 len;
    fileentry *filelist;
};
typedef struct s_directory directory;

struct public packed s_tuple {
    int16 numfiles;
    fileentry *filelist;
};
typedef struct s_tuple tuple;

struct public packed s_filedescriptor {
    ptr inono;
    int32 offset;
    int16 flags;
    bool inuse;
    filesystem *fs;
};
typedef struct s_filedescriptor filedesc;

#define indestroy(f,p) (bool)inunalloc((f), (p))
#define filename2low(x) tolowercase($1 (x))

extern filedesc fdtable[MaxOpenFiles];

/* internal -> private? */
public   filesystem *fsformat(disk*,bootsector*,bool);
internal bitmap *mkbitmap(filesystem*,bool);
internal int16 bitmapalloc(filesystem*,bitmap*, bool);
internal void bitmapfree(filesystem*,bitmap*,int16);
internal void fsshow(filesystem*,bool);
internal inode *findinode(filesystem*,ptr);
internal filename *str2file(int8*);
internal int8 *file2str(filename*);
internal filesystem *fsmount(int8);
internal void fsunmount(filesystem*);

internal ptr increate(filesystem*,filename*,type);
public ptr inalloc(filesystem*);
public bool inunalloc(filesystem*,ptr);
internal ptr fssaveinode(filesystem*,inode*,ptr);
// public fileinfo *fsstat(path*);
public fileinfo *fsstat(filesystem*,ptr);

public bool validfname(filename*, type);
private bool validpname(int8*);
public bool validchar(int8);
private bool validchar_(int8);
internal void showpath(const path*);
internal ptr path2inode(path*);
internal ptr buildpath(path*);
public ptr linkentry(path *pp, filename *name, int16 idx, type inotype);

internal ptr read_dir(filesystem*,ptr,filename*);
public path *mkpath(int8*,filesystem*);
internal tuple *mkfilelist(filesystem*,inode*);

