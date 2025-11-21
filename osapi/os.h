/* os.h */
#pragma once


typedef unsigned char int8;
typedef unsigned short int int16;
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


/* write 1 char */
public bool load(fd, int8);

/* read 1 char */
public int8 store(fd);

public void init();