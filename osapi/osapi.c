/* osapi.c */
#define Library
#include "omnistd.h"
#include "os.h"
#include "osapi.h"
#include "fs.h"

private fd fds[256];


internal int8 *strnum(int8 *str, int8 num) {
    static int8 buf[256];
    int16 n;
    int8 c;
    
    n = stringlen(str);
    if (!n) 
        return str;
    else if  (n > 250) return str;
    else if  (num > 9) return str;
   
   /* 012 */ 
   /* abc\0 */
   /* len=3 */
   
   zero(buf, 256);
   copy(buf, str, n);
    
   c = num+48;
   buf[n++] = c;
   buf[n] = 0;
    
   return buf;
}

/*
 * fd=0 -> error
 * fd=1 -> stdin
 * fd=2 -> stdout
 */

private bool isopen(fd file) {
    signed int posixfd;
    struct stat _;
    
    if (file < 3) return false;
    
    posixfd = getposixfd(file);
    if (!posixfd) return false;
    
    if ((fstat(posixfd, &_)) == -1) return false;
    return true;
}

public bool load(fd file, int8 c) {
    int8 buf[2];
    signed int n;
    signed int posixfd;
    
    asserted_initialized();
    
     if (file > 1)
         if (!isopen(file)) 
             reterr(ErrBadFD);
     
     posixfd = getposixfd(file);
     if (!posixfd) reterr(ErrBadFD);
     
     posixfd = (posixfd == 1) ? 0
         : (posixfd == 2) ? 1
         : (posixfd);
     
     *buf = *(buf + 1) = (int8)0;
     *buf = c;
     n = write(posixfd, $c buf, 1);
     if (n != 1) reterr(ErrIO);
     
     return true;
}

public int8 store(fd file) {
    int8 buf[2];
    signed int n;
    signed int posixfd;
    
    asserted_initialized();

    
     if (file > 3)
         if (!isopen(file)) 
             reterr(ErrBadFD);
     
     posixfd = getposixfd(file);
     if (!posixfd) reterr(ErrBadFD);
     
     posixfd = (posixfd == 1) ? 0
         : (posixfd == 2) ? 1
         : (posixfd);
     
     *buf = *(buf + 1) = (int8)0;
     n = read(posixfd, $c buf, 1);
     if (n != 1) reterr(ErrIO);
     
     return (int8)*buf;
}

private void setupfds() {
    zero($1 fds, sizeof(fds));
    zero($1 fdtable, sizeof(fdtable));
    
    fds[0] = 1;
    fds[1] = 2;
    
    return;
}


public void init() {
    errnumber = (int8)0;
    setupfds();
    initialized = true;
    
    return;
}

// todo: 
internal int16 openfiles(disk* dd) {
    return 0;
}

internal void closeallfiles(disk* dd) {
    return ;
}

public void *opendir(int8 *pathstr) {
    path *p;
    inode *ino;
    ptr iptr, tmp;
    int16 n, size;
    filename name;
    directory *dir;
    tuple *tup;
    
    errnumber = ErrNoErr;
    
    if (!pathstr)
        reterr(ErrArg);
    
    p = mkpath(pathstr, (filesystem *)0);
    if (!p)
        throw();
    
    /* (xx) */
    if (!(*p->target.name))
       iptr = 0; 
    else {
        iptr = path2inode(p);
            if (!iptr && errnumber)
                throw();
    }
    printf("opendir\n");
    showpath(p);
    printf("opendir\n");
    
    // iptr = $2 1;
    
    // if (*p->dirpath[0])
    //     for (n=0; *p->dirpath[n]; n++) {
    //         size = sizeof(struct s_filename);
    //         zero($1 &name, size);
    //         size = stringlen(p->dirpath[n]);
    //         if (!size)
    //             break;
        
    //         copy($1 &name.name, $1 p->dirpath[n], size);
    //         tmp = read_dir(p->fs, iptr, &name);
    //         if (!tmp) {
    //             reterr(ErrPath);
    //         }
            
    //         iptr = tmp;
    //     }
        
    // tmp = read_dir(p->fs, iptr, &p->target);
    // if (!tmp)
    //     reterr(ErrNotFound);
    // else
    //     iptr = tmp;
    ino = findinode(p->fs, iptr);
    if (!ino) {
        reterr(ErrInode);
    }
    
    if (ino->validtype != TypeDir) {
        if (ino)
            destroy(ino);
        
        reterr(ErrBadDir);
    }
    /*
        int16 drive;
        filesystem *fs;
        ptr inode;
        filename *dirname;
        int16 len;
    */ 
    size = sizeof(struct s_directory);
    dir = (directory *)alloc(size);
    if (!dir) {
        if (ino)
            destroy(ino);
        
        reterr(ErrNoMem);
    }
    zero($1 dir, size);

    dir->drive = p->fs->dd->drive;
    dir->fs = p->fs;
    dir->inode = iptr;
    copy($1 &dir->dirname, $1 &p->target, $2 8);
    
    tup = mkfilelist(dir->fs, ino);
    if (!tup) {
        if (ino)
            destroy(ino);
        throw();
    }
    dir->len = tup->numfiles;
    dir->filelist = tup->filelist;
    
    if (tup)    
        destroy(tup);
    if (ino)    
        destroy(ino);
    
    return $v dir;
}

public ptr makedir(int8 *pathstr ) {
    path *pp; /* path pointer */
    ptr idx;
    int8 buf[256], buf1[256], tgt[256]; 
    int8 *p;
    filename name;
    int16 size;
    
    errnumber = ErrNoErr;
      
    zero($1 buf, 256);
    zero($1 tgt, 256);
    stringcopy($1 buf, $1 pathstr, 255);
    stringcopy($1 buf1, $1 pathstr, 255);
    
    size = stringlen(buf);
    while (size > 1 && buf[size-1] == '/')
        size--;
    if (size < stringlen(buf))
        buf[size] = (int8)0;

 
    
    p = findcharr(buf, '/');
    if (!p)
        reterr(ErrPath);
    
    *p++ = 0;
    stringcopy($1 tgt, p, 255);
    p = buf;
    pp = mkpath(p, (filesystem *)0);

    if (!pp)
        throw();
    if (!(*pp->target.name)) 
       idx = 0;
    else {
        idx = path2inode(pp);
        if (!idx && errnumber)
            reterr(ErrPath);
    } 

    pp = mkpath(buf1, (filesystem *)0);
    if (!pp)
        throw();
    
    size = sizeof(struct s_filename);
    zero($1 &name, size);
    size = stringlen(tgt);
    copy($1 &name.name, tgt, $2 size);
    
    return linkentry(pp, &name, idx, TypeDir);
}

public ptr makedir_p(int8 *pathstr) {
    /*
        mkdir -p /ddos/dir_depth_1/dir_depth_2/dir_depth_3/
    */
    path *pp;
    ptr idx, idx2, ret;
    int8 buf[256], tgt[256];
    int16 size;

    errnumber = ErrNoErr;

    zero($1 buf, 256);
    zero($1 tgt, 256);
    stringcopy($1 buf, $1 pathstr, 255);
    stringcopy($1 tgt, $1 pathstr, 255);
    

    size = stringlen(buf);
    while(size > 1 && buf[size-1] == '/') size--;
    if (size< stringlen(buf)) buf[size] = (int8)0;

    pp = mkpath(buf, (filesystem *)0);
    if (!pp)
        throw();

    idx = buildpath(pp);
    if (errnumber)
        reterr(ErrPath);
    ret = makedir(tgt);
    if (!ret)
        reterr(ErrPath);
    return ret;
}

public ptr touch(int8 *pathstr) {
    path *pp; 
    ptr idx;
    int8 buf[256], buf1[256], tgt[256];
    int8 *p;
    int16 size;
    filename *name;
    
    errnumber = ErrNoErr;

    zero($1 &buf, 256);
    zero($1 &tgt, 256);
    stringcopy($1 buf, $1 pathstr, 255);
    stringcopy($1 buf1,$1 pathstr, 255);

    /* (xx) --> maybe i should track / in the end of the pathstr*/ 
    size = stringlen(buf);
    if (buf[size-1] == '/')  reterr(ErrPath);
    
    p = findcharr(buf, '/');
    if (!p)
        reterr(ErrPath);
    
    *p++ = 0;
    stringcopy($1 tgt, $1 p, 255);
    p = buf; 
    pp = mkpath(p, (filesystem *)0);
    if (!pp)
        throw();
    if (!(*pp->target.name))
        idx=0;
    else {
        idx=path2inode(pp);
        if (!idx &&  errnumber)
            reterr(ErrPath);
    }
    
    pp = mkpath(buf1, (filesystem *)0);
    if (!pp)
        throw();
    
    size = sizeof(struct s_filename);
    zero($1 &name, size);
    size = stringlen(tgt);
    name = str2file(tgt);
   
   return linkentry(pp, name, idx, TypeFile);
}

public filedesc fdtable[MaxOpenFiles];
public sint16 fileopen(int8 *pathstr, int16 flags) {
    path *pp;
    int8 buf[256], buf1[256];
    ptr idx;
    int16 n;
    
    errnumber = ErrNoErr;
   
    stringcopy($1 buf, $1 pathstr, 255);
    stringcopy($1 buf1, $1 pathstr, 255);
    pp = mkpath(buf, (filesystem *)0);
    if (!pp)
        reterr(ErrPath);
    
    idx = path2inode(pp);
    if (!idx && (flags & O_CREAT)) {
        idx = touch(buf1);
        if (!idx) reterr(ErrPath);
    }
    else if (!idx)
        reterr(ErrInode);
    
    for (n=0; n<MaxOpenFiles; n++)
        if (!fdtable[n].inuse)
            break ;
    
    if (n<MaxOpenFiles && !fdtable[n].inuse) {
        fdtable[n].inono = idx;
        fdtable[n].offset = 0;
        fdtable[n].flags = flags;
        fdtable[n].inuse = true;
        fdtable[n].fs = pp->fs;
        
        return n;
    }
    return (-1);
}

public sint16 filewrite(int16 fd, int8* str, int16 size) {
    ptr inono, blockno, blockidx;
    inode *ino;
    int32 offset;
    fsblock bl;
   
    errnumber = ErrNoErr;
    
    if (size > 512)
        reterr(ErrArg);
    
    if (fd < 0 || fd >= 256 || !fdtable[fd].inuse)
        reterr(ErrBadFD);
    
    if (!fdtable[fd].inono || !fdtable[fd].fs)
        reterr(ErrInode);
    
    inono  = fdtable[fd].inono;
    offset = fdtable[fd].offset;
    
    blockidx = offset / 512;
    
    ino = findinode(fdtable[fd].fs, inono); /* (destroy needed) */
    if (!ino)
        reterr(ErrInode);
    
    if (!ino->direct[blockidx]) {
        blockno = bitmapalloc(fdtable[fd].fs, fdtable[fd].fs->bitmap);
            if (!blockno)
                reterr(ErrNoMem);
        ino->direct[blockidx] = blockno;
    }
    else blockno = ino->direct[blockidx];
    
    zero($1 &bl, 512);
    copy($1 &bl.data[offset % 512], $1 str, $2 size);
    dwrite(fdtable[fd].fs->dd, &bl.data, blockno);
   
    ino->size = offset + size;
    if (!fssaveinode(fdtable[fd].fs, ino, inono)) 
        reterr(ErrInode);
    fdtable[fd].offset += size;
    
    destroy(ino);
    
    return size;
} 