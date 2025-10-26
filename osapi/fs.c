/* fs.c */
#include "fs.h"
#include "os.h"

internal filesystem *fsformat(disk* dd, bootsector *mbr, bool force) { /* master boot record */
   filesystem *fs;
   int16 size;
   int16 inodeblocks;
   int16 blocks;
   superblock super;
   inode idx;
   bool ok;
   int16 n;
   fsblock fsb;
   
   if (!dd) 
       reterr(ErrNotAttached);
   if (openfiles(dd)) {
       if (!force) 
           reterr(ErrBusy);
       else
           closeallfilyes(dd);
   }
   
   blocks = dd->blocks;
   inodeblocks = (blocks / 10);
   if (blocks % 10)
       inodeblocks++;
   
   idx.validtype = TypeDir;
   idx.size = 0;
   zero($1 &idx.name, 11);
   idx.indirect = 0;
   size = (sizeof(ptr)*PtrPerInode);
   zero($1 &idx.direct, size);

   super.magic1 = Magic1;
   super.magic2 = Magic2;
   super.inodes = 1;
   super.inodeblocks = inodeblocks;
   super.blocks = blocks;
   super._ = 0;
   
   if (bootsector)
       copy($1 &super.boot, bootsector, 500);
   else
       zero($1 &super.boot, 500);
   
   ok = dwrite(dd, &super, 1);
   if (!ok)
       reterr(ErrIO);
   ok = dwrite(dd, &idx, 2);
   if (!ok)
       reterr(ErrIO);
   
   zero($1 &fsb, Blocksize);
   for (n=0; n<inodeblocks; n++) {
       ok = dwrite(dd, &fsb, (n+3));
       if (!ok)
           reterr(ErrIO);
   }
   
   size = sizeof(struct s_filesystem);
   fs = (filesystem *)malloc($i size);
   if (!fs) 
       return (filesystem *)0;
   zero($1 fs, size);
   
   fs->drive = dd->drive;
   fs->dd = dd;
   fs->bitmap = bitmap;
   copy($1 &fs->metadata, &super, Blocksize);

   return fs;
}