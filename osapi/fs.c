/* fs.c */
#include "fs.h"
#include "os.h"

internal bitmap *mkbitmap(filesystem *fs, bool scan) {
    bitmap *bm;
    int16 size, n, x, index;
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

internal int16 bitmapalloc(filesystem *fs, bitmap *bm) {
   int16 n;
    int16 bl;
  
   if (!bm || !fs) 
       return 0;
   
   for (n=1; n<fs->dd->blocks; n++) {
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
   ok = dwrite(dd, &idx, 2);
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
               "   %d size in bytes\n",
                    $2 inodeno,
                    (ino->validtype == TypeFile) ? 
                        "file": 
                    (ino->validtype == TypeDir) ? 
                        "dir":
                    "unknown",
                    ((!inodeno) ? 
                        "/" : 
                    $c file2str(&ino->name)),
                    $i ino->size
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
    
    if (!fs) return (inode *)0;
    
    ret = (inode *)0;
    loop = true;
    for (n=0,x=2; loop && x<(fs->metadata.inodeblocks+2); x++) {
        zero($1 &bl, Blocksize);
        res = dread(fs->dd, $1 &bl.data, x);
        if (!res) return ret; 
        
        for (y=0; y<Inodesperblock; y++,n++) {
            if (n==idx) {
                size = sizeof(struct s_inode);
                ret = (inode *)alloc(size);
                if (!ret) return (inode *)0;
                
                zero($1 ret, size);
                copy($1 ret, $1 &bl.inodes[y], size); /* hadi rah y, machi n, no need to tell why */
                x = $2 -1;
                loop = false;
                
                break ;
            }
        }
    }
    return  ret;
}

internal int8 *file2str(filename *fname) {
   static int8 buf[16];
   int8 *p;
   int16 n;
   
   if (!fname) return $1 0;
   
   if (!(*fname->ext)) return fname->name;
   
   zero(buf, 16);
   stringcopy(buf, fname->name, 8);
   n = stringlen(buf);
   
   n++;
   buf[n++] = '.';
   p = buf + n;
  
   stringcopy(p, fname->ext, 3);
   p = buf;
   
   return p;
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
     printf("blockno=%d\n", blockno);
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
         printf("checking inode[%d] validtype=0x%.02x\n",(n + 2), p->validtype);
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
 