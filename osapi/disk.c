/* disk.h */
#include "disk.h"
#include "os.h"
#include <fcntl.h>


internal bool attached;
/* 
    struct internal packed s_disk {
    int32 fd;
    int16 blocks;
    int8 drive:2;
};
    typedef struct internal packed s_disk disk;
*/
internal void dshow(disk *dd) {
   if (dd)
       printf(
           "drive 0x%.02hhx\n"
           " fd=%d\n"
           " blocks=%d\n\n",
            (char) dd->drive, $i dd->fd,  dd->blocks
       );
   return ;
}

public void dinit(void) {
    disk *dd[2];
    block bl;
   
    for (int n = 0; n < 512; n++) bl[n] = 0x00;
    attached = 0;
    *dd = dattach(1);
    *(dd+1) = dattach(2);
   
    dshow(*dd);
    dshow(dd[1]);
   
    bool x = dwrite(*dd, &bl, 8);
    printf("x=%s\n", (x)?"true":"false");
    //printf("0x%.02hhx\n", (char)(bl[500]));
    
    ddetach(*dd);
    ddetach(*(dd+1));
    
    return ;
}

internal void ddetach(disk *dd) {
   int8 x; 
    
    if (!dd) return ;
    
    close(dd->fd);
    x = ~(dd->drive) & attached;
    attached = x;
    destory(dd);
    
    return;
}

internal disk *dattach(int8 drive) {
    disk *dd;
    int16 size;
    int8 *file;
    signed int tmp;
    struct stat sbuf;
    
    if ((drive==1) || (drive==2));
    else return (disk*)0;
    
    if (attached & drive)
        return (disk *)0;
    
    size = sizeof(struct internal packed s_disk);
    dd = (disk *)malloc($i size);
    if (!dd)
        return (disk *)0;
    
    zero($1 dd, size);
    file = strnum(Basepath, drive);
    tmp = open($c file, O_RDWR);
    if (tmp < 3) {
        destory(dd);
        return (disk *)0;
    }
    dd->fd = $4 tmp;
    
    tmp = fstat($i dd->fd, &sbuf);
    if (tmp || !sbuf.st_blocks) {
        close(dd->fd);
        destory(dd);
        
        return (disk *)0;
    }
    
    dd->blocks = $2 (sbuf.st_blocks-1);
    dd->drive = drive;
    attached |= drive;
    
    return dd;
}