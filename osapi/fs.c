/* fs.c */
#include "fs.h"
#include "omnistd.h"
#include "os.h"

internal bitmap *mkbitmap(filesystem *fs, bool scan) {
    bitmap *bm;
    int16 size, n, x, index = 0;
    fsblock block;
    bool ret, valid;

    if (!fs->dd)
        return (bitmap *)false;

    size = (fs->dd->blocks / 8);
    if (fs->dd->blocks % 8)
        size++;
    bm = (bitmap *)alloc(size);
    if (!bm)
        return (bitmap *)false;
   zero($1 bm, size);

    if (!scan)
        return bm;

    for (n=2; n<=(fs->metadata.inodeblocks+1); n++) {
        zero($1 &block, Blocksize);
        ret = dread(fs->dd, &block, n);
        if (!ret) {
            destroy(bm);
            return (bitmap *)false;
        }

        for (x=0; x < Inodesperblock; x++) {
            valid = (bool)(block.inodes[x].validtype & 0x01);
            if (valid)
                setbit($1 bm, index, true);
            else
                setbit($1 bm, index, false);
            index++;
        }
    }
    return bm;
}

internal int16 bitmapalloc(filesystem *fs, bitmap *bm, bool datablock) {
   int16 n;
   int16 bl;

   if (!bm || !fs)
       return 0;

   for (n=(1+fs->metadata.inodeblocks) ? (datablock) : 1; n<fs->dd->blocks; n++) {
       if (!getbit($1 bm, n)) {
           setbit($1 bm, n, true);
           bl = (n+1);

           return bl;
       }
   }
   return 0;
}

internal void bitmapfree(filesystem *fs, bitmap *bm, int16 bl) {
   int16 n;

   if (!bm || !fs)
       return ;

   n = (bl - 1);
   setbit($1 bm, n, false);

   return ;
}

/*
 * Format disk: write superblock, clear inode table, initialize root dir
 */
public filesystem *fsformat(disk* dd, bootsector *mbr, bool force) { /* master boot record */
   filesystem *fs;
   int16 size, inodeblocks, blocks, n;
   superblock super;
   inode idx;
   bool ok;
   fsblock fsb;
   bitmap *bm;

   if (!dd)
       reterr(ErrNotAttached);
   if (openfiles(dd)) {
       if (!force)
           reterr(ErrBusy);
       else
           closeallfiles(dd);
   }

   /* Calculate inode space: 10% of disk space blocks, rounded up */
   blocks = dd->blocks;
   inodeblocks = (blocks / 10);
   if (blocks % 10)
       inodeblocks++;

   /* Setup root dir inode (empty, no data blocks) */
   idx.validtype = TypeDir;
   idx.size = 0;
   zero($1 &idx.name, 11);
   idx.indirect = 0;
   size = (sizeof(ptr)*PtrPerInode);
   zero($1 &idx.direct, size);

   /* Initialize superblock */
   super.magic1 = Magic1;
   super.magic2 = Magic2;
   super.inodes = 1; // kaina root dir
   super.inodeblocks = inodeblocks;
   super.blocks = blocks;
   super._ = 0;

   if (mbr)
       copy($1 &super.boot, $1 mbr, 500);
   else
       zero($1 &super.boot, 500);

   /* Write superblock to block 1 */
   ok = dwrite(dd, &super, 1);
   if (!ok)
       reterr(ErrIO);
   /* Write root inode to block 2 */
   /* (xx) */
   zero($1 &fsb, Blocksize);
   copy($1 &fsb.inodes[0], $1 &idx, $2 sizeof(inode));
   ok = dwrite(dd, &fsb.data, 2);
   if (!ok)
       reterr(ErrIO);

   /* Create  in-memory filesystem structure */
   zero($1 &fsb, Blocksize);
   for (n=0; n<inodeblocks; n++) {
       ok = dwrite(dd, &fsb, (n+3));
       if (!ok)
           reterr(ErrIO);
   }

   size = sizeof(struct s_filesystem);
   fs = (filesystem *)alloc(size);
   if (!fs)
       reterr(ErrNoMem);
   zero($1 fs, size);

   fs->drive = dd->drive;
   fs->dd = dd;
   copy($1 &fs->metadata, $1 &super, Blocksize);

   bm = mkbitmap(fs, false);
   size =
       1 // superblock
       + 1 // direcotry /
       + fs->metadata.inodeblocks; // # of inode blocks;
    for (n=0; n<=size; n++)
        setbit($1 bm, n, true);

    fs->bitmap = bm;

    return fs;
}

internal void fsshow(filesystem *fs, bool showbm) {
    int8 drivechar;
    ptr inodeno, i;
    inode *ino;

    if (!fs) return ;

    drivechar = (fs->drive == 1) ?
                    (int8)'c' :
                (fs->drive == 2) ?
                    (int8)'d' :
                (int8)'?';
    printf("\n\nDisk 0x%.02hhx, mounted on %c:\n", (char)fs->drive, (char)drivechar);
    printf("   %d total blocks, 1 superblock and %d inode blocks\n"
        "   Containing %d inodes\n\n",
        $i fs->metadata.blocks, $i fs->metadata.inodeblocks, $i fs->metadata.inodes);

   for (inodeno=0; inodeno<fs->metadata.inodes; inodeno++)  {
       ino = findinode(fs, inodeno);
       if (!ino) break ;

        if ((ino->validtype & 0x01))
           printf("Inode %d is valid (type=%s)\n"
               "   filename is %s\n"
               "   %d size in bytes\n"
               "   inodeblocks: %d\n",
                    $2 inodeno,
                    (ino->validtype == TypeFile) ?
                        "file":
                    (ino->validtype == TypeDir) ?
                        "dir":
                    "unknown",
                    ((!inodeno) ?
                        "/" :
                    $c file2str(&ino->name)),
                    $i ino->size,
                    fs->metadata.inodeblocks
           );
   }
   if (showbm) {
       printf("bitmap=");
       fflush(stdout);
       for (i=0; i<fs->dd->blocks; i++) {
           if (getbit($1 fs->bitmap, i)) printf("1");
           else  printf("0");

           if (i>0) {
               if (!(i%8)) printf(" ");
               if (!(i%64)) printf("\n");
           }
       }
       printf("\n\n");
   }
}

internal inode *findinode(filesystem *fs, ptr idx) {
    fsblock bl;
    int16 n, size;
    bool res, loop;
    inode *ret;
    ptr x,y;

    errnumber = ErrNoErr;
    if (!fs) 
        reterr(ErrArg);

    ret = (inode *)0;
    loop = true;
    for (n=0,x=2; loop && x<(fs->metadata.inodeblocks+2); x++) {
        zero($1 &bl, Blocksize);
        res = dread(fs->dd, $1 &bl.data, x);
        if (!res) 
            reterr(ErrIO);

        for (y=0; y<Inodesperblock; y++,n++) {
            if (n==idx) {
                size = sizeof(struct s_inode);
                ret = (inode *)alloc(size);
                if (!ret) 
                    reterr(ErrNoMem);

                zero($1 ret, size);
                copy($1 ret, $1 &bl.inodes[y], size); /* hadi rah y, machi n, no need to tell why */
                x = $2 -1;
                loop = false;

                break ;
            }
        }
    }
    if (!ret)
        reterr(ErrNotFound);
    return  ret;
}

internal int8 *file2str(filename *fname) {
   static int8 buf[16];
   int8 *p;
   int16 n;

   if (!fname) return $1 0;


   zero(buf, 16);
   copy($1 buf, $1 fname->name, $2 8);

   if (!(*fname->ext)) {
       p = buf;
       return p;
   }
   n = stringlen(buf);
   buf[n++] = '.';
   p = buf + n;

   copy(p, fname->ext, 3);
   p = buf;

   return p;
}

internal filename *str2file(int8 *str) {
    int8 *p;
    static filename name;
    filename *fnptr;
    int16 size;

    printf("\nstr2file: str='%s'\n", $c str);

    if (!str)
        reterr(ErrArg);

    size = sizeof(struct s_filename);
    zero($1 &name, size);
    p = findcharl(str, '.');
    if (!p)
        copy($1 &name.name, $1 str, $2 stringlen(str));
    else {
        copy($1 &name.ext , (p+1), $2 3);
        *p = (int8)0;
        stringcopy($1 &name.name, $1 str, $2 8);
    }
    fnptr = &name;

    return fnptr;
}

 public filesystem *FSdescriptor[Maxdrive];
 internal filesystem *fsmount(int8 drive) {
    ptr idx;
    disk *dd;
    filesystem *fs;
    int16 size;

    if (drive > Maxdrive) return (filesystem *)0;
    idx = (drive-1);
    dd = DiskDescriptor[idx];
    if (!dd) return (filesystem *)0;

    size = sizeof(struct s_filesystem);
    fs = (filesystem *)alloc(size);
    if (!fs) return (filesystem *)0;

    zero($1 fs, size);

    /*
     struct internal packed s_filesystem {
         int8 drive:2;
         disk *dd;
         bool *bitmap;
         superblock metadata;
     };
     typedef struct s_filesystem filesystem;
     */

     fs->drive = drive;
     fs->dd = dd;
     fs->bitmap = mkbitmap(fs, true);
     if (!fs->bitmap) {
         destroy(fs);
         return (filesystem *)0;
     }
     if (!dread(fs->dd, &fs->metadata, 1)) {
         destroy(fs);
         return (filesystem *)0;
     }

     kprintf("Mounted disk 0x%x on drive %c: ",
         drive, (drive == 1) ? 'c' : (drive == 2) ? 'd' : '?'
     );

     FSdescriptor[idx] = fs;

    return fs;
 }

 internal void fsunmount(filesystem *fs) {
    ptr idx;
    int16 drive;

    if (!fs) return ;
    drive = fs->dd->drive;
    idx = (fs->dd->drive-1);
    FSdescriptor[idx] = (filesystem *)0;
    destroy(fs);

    kprintf("Unmounted drive %c: ",
        drive, (drive == 1) ? 'c' : (drive == 2) ? 'd' : '?'
    );
    return ;
 }

 internal ptr fssaveinode(filesystem *fs, inode *ino, ptr idx) {
     fsblock bl;
     ptr blockno, blockoffset;
     int16 size;
     bool ret;
     if (!fs || !ino)
         return 0;

     /* Kain Mushkil hna, if something happened, tryto change 3 -> 2,
      * since block 0 is reservec,
      * and the 1 is the superblock,
      * 2 root inode (directory) :
      * |
      * |__> Al7mar, machi lblock 2 dial root inode, rah block 2
      *     fih 16 inode, ou linode lwla dial root directory, ye3ni
      *     ba9in 15, ye3ni a dada lfahim, rah blockno = (idx / 16) + 2 machi + 3
      *     we are still on the block two a chrif
      */
     blockno = (idx / 16) + 2;
     blockoffset = (idx % Inodesperblock);
     // printf("blockno=%d\n", blockno);
     ret = dread(fs->dd, &bl.data, blockno);
     if (!ret)
         return 0;
     size = sizeof(struct s_inode);
     copy($1 &bl.inodes[blockoffset], $1 ino, size);


     ret = dwrite(fs->dd, &bl.data, blockno);
     if (!ret)
         return 0;

     return blockno;
 }
 /* Errors NTBI */
 public ptr inalloc(filesystem *fs) {
     ptr idx, n, max;
     inode *p;

     if (!fs)
         return 0;

     max = (fs->metadata.inodeblocks * Inodesperblock);
     for (idx=n=0; n<max; n++) {
         p = findinode(fs, n);
         if (!p)
             break ;
         // printf("checking inode[%d] validtype=0x%.02x\n",(n + 2), p->validtype);
         if (!(p->validtype & 0x01)) {
             idx=n;
             p->validtype = 1;
             if (!(fssaveinode(fs,p,idx))) {
                 idx=0;
                 break ;
             }
             fs->metadata.inodes++;
             dwrite(fs->dd, &fs->metadata, 1);

             break ;
         }
     }
     if (p)
         destroy(p);

     return idx;
 }

 public bool inunalloc(filesystem *fs, ptr idx) {
     inode *p;
     ptr blockno;

     if (!fs)
         return false;

      p = findinode(fs, idx);
      if (!p) {
          destroy(p);
          return false;
      }
      p->validtype = 0x00;
      blockno = fssaveinode(fs, p, idx);
      destroy(p);
      fs->metadata.inodes--;
      dwrite(fs->dd, &fs->metadata, 1);

      return (blockno) ? true : false;
 }

 public bool validfname(filename *name, type filetype) {
    int16 n;
    bool ret;
    int8 *p, *str;

    if (!name || !filetype)
       reterr(ErrArg);
    else if ((filetype == TypeDir) && (*name->ext))
       reterr(ErrFilename);

    n = (filetype == TypeFile) ? 11 : 8;

    for (ret=true, p = $1 name; n;  n--, p++) {
        if (*p == '\0')
            continue;

        if (!validchar(*p)) {
            ret = false;
            break;
        }
    }
    return ret;
 }

 private bool validpname(int8 *name) {
     int16 n;
     bool ret;
     int8 *p;

     if (!name)
         reterr(ErrArg);

     n = stringlen(name);

     for (ret=true, p = $1 name; n;  n--, p++)
         if (!validchar_(*p)) {
             ret = false;
             break ;
     }

     return ret;
 }


 internal ptr increate(filesystem *fs, filename *name, type filetype) {
     ptr idx;
     inode *ino;
     bool valid;
     int16 size;

     errnumber = ErrNoErr;

     if (!fs || !name || !filetype)
         reterr(ErrArg);

     valid = validfname(name, filetype);
     if (!valid)
         reterr(ErrFilename);

     idx = inalloc(fs);
     if (!idx)
         reterr(ErrInode);

     size = sizeof(struct s_inode);
     ino = (inode *)alloc(size);
     if (!ino)
         reterr(ErrNoMem);

     ino->validtype = filetype;
     ino->size = 0;

     size = sizeof(struct s_filename);
     copy($1 &ino->name, $1 name, size);

     valid = (bool)fssaveinode(fs, ino, idx);
     destroy(ino);
     if (!valid)
         reterr(ErrIO);

     return idx;
 }
/*
public fileinfo *fsstat(path* pathname) {
*/
 public fileinfo *fsstat(filesystem *fs, ptr idx) {
     inode *ino;
     fileinfo *info;
     int16 size;

     if (!fs)
         reterr(ErrArg);

     ino = findinode(fs, idx);
     if (!ino)
         reterr(ErrInode);

     size = sizeof(struct s_fileinfo);
     info = (fileinfo *)alloc(size);
     if (!info)
         reterr(ErrNoMem);
     zero($1 info, size);

     info->idx = idx;
     info->size = ino->size;
     destroy(ino);

     return info;
 }


internal ptr read_dir(filesystem *fs, ptr haystack, filename *needle) {
    inode *dir, *child;
    ptr idx, n, blockno;
    fsblock bl;
    bool ret;

    errnumber = ErrNoErr;

    printf("read_dir: searching inode %d for '%s'\n", haystack, file2str(needle));

    if (!fs || !needle)
        reterr(ErrArg);

    dir = findinode(fs, haystack);
    if (!dir)
        reterr(ErrInode);

    if (dir->validtype != TypeDir) {
        destroy(dir);
        reterr(ErrBadDir);
    }

    for (n=0; n<PtrPerInode; n++) {
        idx = dir->direct[n];
        if (!idx)
            continue;

        child = findinode(fs, idx);
        if (!child)
            continue;

        if (cmp($1 needle, $1 &child->name, $2 11)) {
            destroy(dir);
            destroy(child);

            return idx;
        }
        destroy(child);
    }

    if (!dir->indirect) {
        destroy(dir);
        reterr(ErrNotFound);
    }

    blockno = dir->indirect;
    zero($1 &bl, Blocksize);
    ret =  dread(fs->dd, &bl, blockno);
    if (!ret) {
        destroy(dir);
        reterr(ErrNotFound);
    }

    for (n=0; n<PtrPerBlock; n++) {
        idx = bl.pointers[n];
        if (!idx)
            continue;

        child = findinode(fs, idx);
        if (!child)
            continue;

        if (cmp($1 needle, $1 &child->name, $2 11)) {
            destroy(dir);
            destroy(child);

            return idx;
        }
        destroy(child);
    }
    destroy(dir);
    reterr(ErrNotFound);
}

public bool validchar(int8 c) {
    int8 *p;
    bool ret;

    ret = false;
    for (p=ValidChars; *p; p++)
        if (c == *p) {
           ret = true ;
           break;
        }
    
    return ret;
}

private bool validchar_(int8 c) {
    int8 *p;
    bool ret;

    ret = false;
    for (p=ValidPathChars; *p; p++)
        if (c == *p) {
           ret = true ;
           break;
        }

    return ret;
}

/*
 *      c:/ddos/system16/driver.sys
 *      /ddos/system16/driver.sys
 */

private bool parsepath(int8 *str, path *pptr, int16 idx_) {
    int8 *p;
    int idx;

    idx = idx_;
    if (!str || !pptr)
        return false;

    if (idx >= (DirDepth)) {
        *pptr->dirpath[idx] = (int8)0;
        return false;
    }
    
    p = findcharl(str, (int8)'/');
    if (!p) {
        stringcopy($1 pptr->dirpath[idx], $1 str, $2 8);
        idx++;
        *pptr->dirpath[idx] = (int8)0;

        return true;
    }

    *p = 0;
    
    stringcopy($1 pptr->dirpath[idx], $1 str, $2 8);
    p++;
    idx++;

    return parsepath(p, pptr, idx);
}

/*
    filesystem *fs;
    int16 drive;
    filename target;
    int8 dirpath[DirDepth+1][9];
 */

internal void showpath(const path *filepath) {
    int8 *p;
    int16 n;
    
    if (!filepath)
        return ;
    
    printf(
        "filesystem: \t0x%.08x\n"
        "drive: \t0x%.02hhx\n"
        "target: \t%s\n",
             $i filepath->fs, (char)filepath->drive,
                $c file2str(&filepath->target)
    );
    fsshow(filepath->fs,false);
    
    for (p=filepath->dirpath[n=0]; *p; p=filepath->dirpath[++n])
        printf("   %d=%s\n", $i n, $c p);
}

public path *mkpath(int8 *str, filesystem *fs) {
    static path path_;
    path *pptr;
    int8 c, drive;
    int8 *p;
    filename *name;
    int16 size;
    bool ret;

    errnumber = ErrNoErr;
    zero($1 &path_, sizeof(path));

    size = stringlen(str);
    while (size > 1 && str[size-1] == '/')
        size--;
    if (size < stringlen(str))
        str[size] = (int8)0;

    printf("\033[1m" "Daba blati n3ref had str lidakhel -> '%s'\033[0m\n", $c str);

    if (!str || !(*str))
        reterr(ErrArg);
    
    pptr = &path_;

    if (str[1] == ':') {
        c = low(*str);
        if (c < (int8)'a' || c > (int8)'z')
            reterr(ErrDisk);
        drive = (c-0x62);

        str += 2; // c: <- skibi a abdesmi3
    }
    else if (!fs)
        reterr(ErrArg);
    else
        drive = fs->dd->drive;

    if (!validpname(str))
        reterr(ErrPath);

    path_.fs = FSdescriptor[drive-1];
    if (!path_.fs)
        path_.fs = fs;
    if (!path_.fs)
        reterr(ErrDrive);
    if (!drive)
        reterr(ErrDrive);

    path_.drive = drive;
    /* (xx)  <-- handling root directory */
    if (!(*str)) {
        zero($1 &path_.target, sizeof(filename));
        return pptr;
    }
    p = findcharr(str, (int8)'/');
    if (!p)
        reterr(ErrPath);
    p++;

    name = str2file(p);
    if (!name)
        reterr(ErrFilename);

    size = sizeof(struct s_filename);
    copy($1 &path_.target, $1 name, $2 size);
    
    // /ddos
    p--;
    *p = (int8)0;
    printf("str=%s\n", $c str);
    if (!(*str))
        return pptr;
    str++;
    
    p = findcharl(str, (int8)'/');
    if (!p)
        stringcopy($1 &path_.dirpath[0], $1 str, $2 8);
    else {
        ret = parsepath($1 str, &path_, $2 0);
        if (!ret)
            reterr(ErrPath);
    }
    
    return pptr;
}

internal tuple *mkfilelist(filesystem *fs, inode *dir) {
    fileentry *filelist, *entry;
    fileentry arr[MaxFilesPerDir];
    ptr iptr;
    int16 n, size, files = 0;
    inode *ino;
    fsblock bl;
    bool ret;
    tuple *tup;
    
    errnumber = ErrNoErr;
    if (!fs || !dir)
        reterr(ErrInode);
    
    for (n=0; n<PtrPerInode; n++) {
        iptr = dir->direct[n];
        if (!iptr)
            continue;
        
        ino = findinode(fs, iptr);
        if (!ino)
            continue;
        
        files++;
        entry = &arr[files-1];
        
        entry->inode = iptr;
        size = sizeof(struct s_filename);
        copy($1 &entry->name, $1 &ino->name, $2 size);
        entry->size = ino->size;
        entry->filetype = ino->validtype;
        
        destroy(ino);
    }
   
    iptr = dir->indirect;
    if (iptr) {
        zero($1 &bl, Blocksize);
        ret = dread(fs->dd, $1 &bl, iptr);
        if (ret) {
            for (n=0; n<PtrPerBlock; n++) {
                iptr = bl.pointers[n];
                if (!iptr)
                    break;
                
                ino = findinode(fs, iptr);
                if (!ino)
                    continue;
                        
                files++;
                entry = &arr[files-1];
                        
                entry->inode = iptr;
                size = sizeof(struct s_filename);
                copy($1 &entry->name, $1 &ino->name, $2 size);
                entry->size = ino->size;
                entry->filetype = ino->validtype;
                
                destroy(ino);
            }
        }
    }
    size = sizeof(struct s_fileentry) * files;
    filelist = (fileentry *)alloc(size);
    if (!filelist)
        reterr(ErrNoMem);
    zero($1 filelist, size);
    copy($1 filelist, $1 &arr, size);
    
    size = sizeof(struct s_tuple);
    tup = (tuple *)alloc(size);
    if (!tup)
        reterr(ErrNoMem);
    zero($1 tup, size);
    
    tup->numfiles = files;
    tup->filelist = filelist;
    
    return tup;
}

internal ptr path2inode(path *p) {
    ptr iptr;
    int16 size, n, tmp;
    filename name;
    
    /* (xx) */
    iptr = $2 0;
    errnumber = ErrNoErr;
    if (*p->dirpath[0])
        for (n=0; *p->dirpath[n]; n++) {
            size = sizeof(struct s_filename);
            zero($1 &name, size);
            size = stringlen(p->dirpath[n]);
            if (!size)
                break;
        
            copy($1 &name.name, $1 p->dirpath[n], size);
            tmp = read_dir(p->fs, iptr, &name);
            if (!tmp) {
                reterr(ErrPath);
            }
            
            iptr = tmp;
        }
        
    tmp = read_dir(p->fs, iptr, &p->target);
    if (!tmp)
        reterr(ErrNotFound);
    else
        return tmp;
}

internal ptr buildpath(path *p) {
    ptr iptr, idx;
    int16 size, n, i, tmp, blockno;
    filename name;
    inode *ino;
    fsblock bl;
    
    /* (xx) */
    iptr = $2 0;
    errnumber = ErrNoErr;
    if (*p->dirpath[0])
        for (n=0; *p->dirpath[n]; n++) {
            size = sizeof(struct s_filename);
            zero($1 &name, size);
            size = stringlen(p->dirpath[n]);
            if (!size)
                break;
        
            copy($1 &name.name, $1 p->dirpath[n], size);
            tmp = read_dir(p->fs, iptr, &name);
            if (!tmp) {
                idx = increate(p->fs, &name, TypeDir);
                if (!idx)
                    reterr(ErrInode);
                ino = findinode(p->fs, iptr);
                if (!ino)
                    reterr(ErrInode);

                for (i=0; i<PtrPerInode; i++)
                    if (!ino->direct[i])
                        break;
                if (i<PtrPerInode && !ino->direct[i]) { /* Me39ola ytzad check diak i<PtrPerInode*/
                    ino->direct[i] = idx;
                    if (!fssaveinode(p->fs, ino, iptr)) {
                        inunalloc(p->fs, idx);
                        destroy(ino);
                        throw();
                    }
                    destroy(ino);
                }

                else if (!ino->indirect) {
                    blockno = bitmapalloc(p->fs, p->fs->bitmap, false);
                    ino->indirect = blockno;
                    if (!fssaveinode(p->fs, ino, iptr)) {
                        inunalloc(p->fs, idx);
                        destroy(ino);
                        throw();
                    }
                    zero($1 &bl.data, Blocksize);
                    bl.pointers[0] = idx;
                    if (!dwrite(p->fs->dd, &bl.data, blockno)) {
                        ino->indirect = 0;
                        inunalloc(p->fs, idx);
                        bitmapfree(p->fs, p->fs->bitmap, blockno);
                        destroy(ino);

                        throw();
                    }
                    destroy(ino);
                }
                
                else {
                    if (!dread(p->fs->dd, &bl.data, ino->indirect)) {
                        inunalloc(p->fs, idx);
                        destroy(ino);
                        throw();
                    }

                    for (i=0; i<(Blocksize/sizeof(ptr)); i++)
                        if (!bl.pointers[i])
                            break ;
                    if (i<(Blocksize/sizeof(ptr)) && !bl.pointers[i]) {
                        bl.pointers[i] = idx;
                        if (!dwrite(p->fs->dd, &bl.data, ino->indirect)) {
                            inunalloc(p->fs, idx);
                            destroy(ino);
                            throw();
                        }
                        destroy(ino);
                    } 
                    else {
                        inunalloc(p->fs, idx);
                        destroy(ino);
                        reterr(ErrDirFull);
                    }
                }
                iptr = idx;
                continue;
            }
            iptr = tmp;
        }
        
    return iptr;
}

public ptr linkentry(path *pp, filename *name, int16 idx, type inotype) {
    int16 n, i;
    ptr idx2, blockno;
    inode *ino;
    fsblock bl;
    
    
    idx2 = increate(pp->fs, name, inotype);
    if (!idx2)
        reterr(ErrInode);
    
    ino = findinode(pp->fs, idx);
    if (!ino)
        reterr(ErrInode);
    
    for (n=0; n<PtrPerInode; n++) 
        if (!ino->direct[n]) 
            break;
    
    if (n<PtrPerInode && !ino->direct[n]) {
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
        blockno = bitmapalloc(pp->fs, pp->fs->bitmap, false);
        ino->indirect = blockno;
        if (!fssaveinode(pp->fs, ino, idx)) {
            inunalloc(pp->fs, idx2);
            bitmapfree(pp->fs, pp->fs->bitmap, blockno);
            destroy(ino);
            
            throw();
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
    else {
        if (!dread(pp->fs->dd, &bl.data, ino->indirect)) {
            inunalloc(pp->fs, idx2);
            destroy(ino);
            throw();
        }

        for (i=0; i<(Blocksize/sizeof(ptr)); i++)
            if (!bl.pointers[i])
                break ;
        
        if (i<(Blocksize/sizeof(ptr)) && !bl.pointers[i]) {
            bl.pointers[i] = idx2;
            if (!dwrite(pp->fs->dd, &bl.data, ino->indirect)) {
                inunalloc(pp->fs, idx2);
                destroy(ino);
                throw();
            }
            destroy(ino);
            return idx2;
        }
        else {
            inunalloc(pp->fs, idx2);
            destroy(ino);
            reterr(ErrDirFull);
        }
    }
}