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
    ptr idx, idx2, blockno;
    int8 buf[256], tgt[256], tmp[256];
    int8 *p;
    filename name;
    int16 size, n;
    inode *ino;
    fsblock bl;
    
    errnumber = ErrNoErr;
      
    zero($1 buf, 256);
    zero($1 tgt, 256);
    stringcopy($1 &buf, $1 pathstr, 255);
    
    size = stringlen(buf);
    while (size > 1 && buf[size-1] == '/')
        size--;
    if (size < stringlen(buf))
        buf[size] = (int8)0;

 
    
    p = findcharr(buf, '/');
    if (!p)
        reterr(ErrPath);
    
    *p++ = 0;
    stringcopy(tgt, p, 255);
    p = buf;
    stringcopy($1 &tmp, $1 pathstr, 255);
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

    pp = mkpath(pathstr, (filesystem *)0);
    if (!pp)
        throw();
    
    size = sizeof(struct s_filename);
    zero($1 &name, size);
    size = stringlen(tgt);
    copy($1 &name.name, tgt, $2 size);
    
    idx2 = increate(pp->fs, &name, TypeDir);
    if (!idx2)
        reterr(ErrInode);
    
    ino = findinode(pp->fs, idx);
    if (!ino)
        reterr(ErrInode);
    
    for (n=0; n<PtrPerInode; n++) 
        if (!ino->direct[n]) 
            break;
    
    if (!ino->direct[n]) {
        ino->direct[n] = idx2;
        if (!fssaveinode(pp->fs, ino, idx)) {
            inunalloc(pp->fs, idx2);
            destroy(ino);
            throw();
        }
        destroy(ino);
        
        return idx2;
    }
    
    if (!ino->indirect) {
        blockno = bitmapalloc(pp->fs, pp->fs->bitmap);
        ino->indirect = blockno;
        if (!fssaveinode(pp->fs, ino, idx)) {
            inunalloc(pp->fs, idx2);
            bitmapfree(pp->fs, pp->fs->bitmap, blockno);
            destroy(ino);
            
            throw();
        }
    }
        
    zero($1 &bl.data, Blocksize);
    bl.pointers[0] = idx2;
    if (!dwrite(pp->fs->dd, &bl.data, blockno)) {
        ino->indirect = 0;
        inunalloc(pp->fs, idx2);
        bitmapfree(pp->fs, pp->fs->bitmap, blockno);
        destroy(ino);
        
        throw();
    }
    
    destroy(ino);
    
    return idx2;
}

/*
    mkdir -p /ddos/dir_depth_1/dir_depth_2/dir_depth_3/
*/

public ptr makedir_p(int8 *pathstr) {
    path *pp;
    ptr idx, idx2, blockno, ret;
    int8 buf[256], tgt[256];
    int8 *p;
    filename name;
    int16 size, n;
    inode *ino;
    fsblock bl;

    errnumber = ErrNoErr;

    zero($1 buf, 256);
    zero($1 tgt, 256);
    stringcopy($1 &buf, $1 pathstr, 255);
    stringcopy($1 &tgt, $1 pathstr, 255);
    

    size = stringlen(buf);
    while(size > 1 && buf[size-1] == '/') size--;
    if (size< stringlen(buf)) buf[size] = (int8)0;

    pp = mkpath(buf, (filesystem *)0);
    if (!pp)
        throw();

    idx = buildpath(pp);
    if (!idx && errnumber)
        reterr(ErrPath);
    ret = makedir(tgt);
    if (!ret)
        reterr(ErrPath);
    return ret;
    
}