/* os.h */
#pragma once

typedef unsigned char int8;
typedef unsigned short int int16;
typedef signed short sint16;
typedef unsigned int int32;
typedef unsigned long long int int64;
typedef int8 fd;
typedef int8 error;

#define $1 (int8 *)
#define $2 (int16)
#define $4 (int32)
#define $8 (int64)
#define $c (char *)
#define $i (int)
#define $v (void *)

#define bool int8
#define true 1
#define false 0



#define packed __attribute__((packed))
#define public __attribute__((visibility("default")))
#define internal __attribute__((visibility("hidden")))
#define private static

enum public packed {
    ErrNoErr,
    ErrInit,
    ErrIO,
    ErrBadFD,
    ErrNotAttached,
    ErrBusy,
    ErrNoMem,
    ErrArg,
    ErrFilename,
    ErrInode,
    ErrBadDir,
    ErrNotFound,
    ErrDisk,
    ErrPath,
    ErrDrive,
    ErrDirFull,
};

#define reterr(x)   do {\
    errnumber = (x);    \
    return 0;           \
} while (false)
#define throw() return 0
#define alloc(x) malloc($i x)
#define destroy(x) free(x)

#ifdef Library
    public bool initialized = false;
    public error errnumber;
#else
    extern public bool initialized;
    extern public error errnumber;
#endif


public bool load(fd,int8);
public int8 store(fd);
public void init();

public void *opendir(int8*);
public int16 makedir(int8*);
public int16 makedir_p(int8*);
public int16 touch(int8*);

public sint16 fileopen(int8*,int16);
public sint16 filewrite(int16,int8*,int16);