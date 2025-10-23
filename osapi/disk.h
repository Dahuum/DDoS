/* disk.h */

#include <stdio.h>
#include "osapi.h"

#define DriveC      0x01    /* 0001 */
#define DriveD      0x02    /* 0010 */
#define Basepath    $1 "/home/void_id/Desktop/DDoS/drives/disk."

#define Blocksize   512

typedef int8 block[Blocksize];

struct internal packed s_disk {
    int32 fd;
    int16 blocks;
    int8 drive:2;
};
typedef struct internal packed s_disk disk;

internal disk *dattach(int8);
internal void ddetach(disk*);
internal void dshow(disk*);

/* 
bool dread(disk *dd, block *addr, int16 blockno);
bool dwrite(disk *dd, block *addr, int16 blockno);
*/

#define dio(f,d,a,b)    ( \
    (d) && \
    (lseek($i (d)->fd, $i (Blocksize*(b)), SEEK_SET) != -1) && \
    (((f)($i (d)->fd, $c (a), Blocksize) == Blocksize)) \
)
#define dread(d,a,b)        dio(read, d,a,b)
#define dwrite(d,a,b)       dio(write, d,a,b)