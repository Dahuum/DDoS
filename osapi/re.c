#include "fs.h"

public filesystem *fsformat(disk* dd, bootsector* mbr, bool force) {
    /* vars here */
    int16 blocks, inodeblocks, size, n;
    inode idx;
    superblock super;
    bool ok;
    fsblock fsb;
    filesystem *fs;
    
    if (!dd) reterr(ErrNotAttached);
    if (openedfiles(dd)) {
        if (force) closeallfiles(dd);
            else reterr(ErrBusy);
    }
    
    /* Calculate inodes space: 10% of disk blocks rounded up */
    blocks = dd->blocks;
    inodeblocks = (blocks / 10);
    if (blocks % 10) inodeblocks++;
    
    /* Create root dir inode */
    idx.validtype = 0x03;
    idx.size = 0;
    zero($1 &idx.name, 11);
    idx.indirect = 0;
    size = sizeof(ptr) * PtrPerInode;
    zero($1 &idx.direct, size);
  
    /* Initialize Superblock */
    super.magic1 = Magic1; /* 0xdd05 */
    super.magic2 = Magic2; /* 0xaa55 */
    super.inodeblocks = inodeblocks;
    super.inodes = 1; /* root dir exist */
    super.blocks = blocks;
    super._ = 0; /* not used */
    
    if (mbr)
        copy($1 &super.boot, $1 &mbr, 500);
    else 
        zero($1 &super.boot, 500);
    
    /* Write Superblock on block 1 */
    ok = dwrite(dd, &super, 1);
    if (!ok) reterr(ErrIO);
    /* Write Root Dir Inode on block 2 */
    ok = dwrite(dd, &idx, 2);
    if (!ok) reterr(ErrIO);
    
    zero($1 &fsb, Blocksize);
    for (n=0; n<inodeblocks; n++) {
        ok = dwrite(dd, &fsb, n+3);
        if (!ok) reterr(ErrIO);
    }
    
    fs = (filesystem *)alloc(sizeof(struct s_filesystem));
    if (!fs) reterr(ErrNoMem);
    fs->drive = dd->drive;
    fs->dd = dd;
    copy($1 &fs->metadata, $1 &super, (Blocksize)); /* why here size of block size (i was doing sizeof(struct s_superblock)) */
    fs->bitmap = 0;
    return fs;
}

/*
 * S1:
 * Q1: first of all format() is only called once, just to Initialize the disk, 
 * and of course if the user (me in this case) wanted to erase and format;
 * about computer crasheds today, tomorow once the boot or anything, of course 
 * it use the mount not the format, and i think we don't need even to explain it
 * (because we gonna read from the disk normnally using the mount and read everything, and nothing 
 * happen to super block, from my code i think there is no read or right on it right ? since we 
 * only have control over the root dir and up ? i' m not sure but i think)
 * 
 * Q2: simply erased, cause format create new superblock, empty root dir, and zero every else node;
 * 
 * Q3: simply this line --> if (!dd) return (filesystem *)0; (there is no allocated dd
 * i 'm not deleting this cause i' m wrong, and  that's what i though) but the dd is well Initialized
 * cause it's only the process of opening a file for RD_WR; so the real failure will be here 
 * ```
 * if (!dread(fs->dd, &fs->metadata, 1)) {
 * destory(fs);
 * return (filesystem *)0;
 * }
 * ```
 * 
 * S2:
 * Q4: That's really an tricky question to be honest, cause i know that it's (in my case)
 * it's only an program so it only exist on the RAM, but i think the answer is really more 
 * deep, cause is it about later on when the os will be completed or right now?
 * 
 * Q5: who gave the user to edit it, i' m just jockign for sure? but editing inodes is weird
 * and really make me think very well ? but i will go to it straight editing using disk write(dwrite)  
 * will defently edit it also on the disk, so absolutly on the next mount will be edit, 
 * unless if there is checks or re calculation and re initializatrion of bitmaps
 * 
 * Q6:  hahah no worries, first  check on fsfomrat is 
 * ```
 * if (!dd) 
 * reterr(ErrNotAttached);
 * ```
 * hahaha and running fsmount(1), it's easy also hahaha,  cause also there is this check on the first
 * lines --> if (!dd) return (filesystem *)0;
 * 
 * S3:
 * Q7: i' m little bit confused right now, (hahah),  how many blocks it scan i think mhm the superblock
 * idk tbh, also hte to be honest htis question confused me a little bit, 
 * 
 * Q8: easy that question, to know if the inode is reallyu, no before you should know that i have three
 * types of the inode, 0x00 TypeNonValid (unused inode), 0x01 TypeFile, 0x03 TypeDir, the & (which is
 * the and operator, simple do an and operations between all this nums and the 1 (0x01), if it's 
 * empty give 0 cause and with 0 is 0, and the others 3 1 and 1 are 1 so they are used)
 * 
 * Q9: if we don't scan so we will not know which one's are really used right ? so if !scan and since 
 * we zero all the bitmap (false it all)  then the whole bitmp returned without scan will be false
 * 
 * S4: 
 * Q10: yes it will work (ofc if disk.1 exist on /drives), fs1 is the filesystem created and returned 
 * by fs1, and of course fs1 == fs2 cause fs2 is just mounting dd and getting fs1 that was originally 
 * created on the format
 * 
 * Q11: technically yes, unless if we want to access to read or write the dd ? (m i wrong)  ?
 * 
 * S5:
 * Q12: based on my calcualtions i do 10%, so 1000 will give 100 inodes in total ? i' m not susre
 * we had this problem in previous questions, i' m not sure, cause no calculations re do it, cause 
 * if i add it nohting will happen since the inode blocks is just like my choice, i want it 10%, 
 * i can make it more unless
 * 
 * Q13: wait what ? where is this inodeblocks+2 ? we loop from the third block skipping the reserved
 * one and sb and root ? and we g zeroing everything, in the bitmap in the question before i still can't
 * get it well so
 * 
 * Q14: easy man:  
 * if (openfiles(dd))
 *      if (!force) 
 *          reterr(ErrBusy);
 *  since there is open files and force == 0 so there is no way the format will work
 *  
 * S6:
 * Q15: aren't we creating new ram var for the fs ? i don't know to be honest, cause 
 * we don't have the fs once the unmount so we need that to create it (need more on that also please)
 * 
 * Q16: why one array ? aren't they two different types ? so we store two fsd's in one, and two
 * dd's on theyre own one, make them both global access from any where ? 
 * 
 * Q17: hahaha that make sence in all quesiton, and specialy hte question why why we are calling it with true
 * on the fsmount, when we create the bimap on the foirmat we set it to false no scan, also and it's not commented
 * anymore for sure 
 * bm = mkbitmap(fs, false);
 * size = 
 *    1 // superblock
 *    + 1 // direcotry /
 *    + fs->metadata.inodeblocks; // # of inode blocks;
 * for (n=0; n<=size; n++)
 *     setbit($1 bm, n, true);
 * 
 * fs->bitmap = bm;
 * and the size that we are giving it is normal, as i think right ? fuck that make sense again when you asked me 
 * about why we start the loops with n=1 to n+1, fuck, it's because to skip the others for sure and zero the others
 * also we set everythign to true, it's normal to get 1 or true, make sense on previous quesion, i' m not editing 
 * nothing 
 * 
 * Q18: to be honest, i don't see any problem with it, in ddosutil i should edit it, so i should it on main, also just
 * edit the MaxDrive = 0x02 to 0x03 ? that's it since there is loops and everything working normally 
 * 
 * Q19: even if i didn't get the thing very well, cause things are not written well, first of all i do write in 
 * the offset 510 512 so of course the format, which is not implemented right now will be an reading or
 * identifying error, i dohn't know tbh, that's the main reason for me
 * 
 * Q20: wow that made me really think to be honest, mhm, maybe i don't know ? what could be the problem ? did he 
 * edited something or touched something should not be touched , of course mounting to the correct disk and not 
 * finding the previous or the existing files mean it might be an disk error but it mounted correctly
 * i' m not sure what's the error
 *
 * S8: 
 * Q21: i will do my best and just write what i really know, since as you know things are current developpement, nothing
 * done yet, so i may not know all the steps, but:
 * formatting, start by having an open disk ready for read and write operations.
 * formatting mean creating the 3 main blocks and zeroing all the inode blocks.
 * what happens in ram: i' m not sure ? to be honest while formatting, 
 *  in disk: as i said i think
 * when data is written: when mounted i can't get the orientation of the question
 * files, and every data written in the disk for sure, 
 * of courese it live in the ram, even i don't know exactly how but yes 
 */
 
 
 Wellefna sehir lil ana w lgemra mkiyfin
 Khok ghadi wakha ymil, ghaybano chkon li mziyfin
 Ghaybano chkon li real, kamlin 3el l3a9a mtayfin
 Ga3 tme3na f dollar bills w f Porto Rico mseyfin
 Nti li 9telti 9elbi wakha kan bohdk kaybghik
 Ana li 9telt nniya w 7alef 3emro y3awed yti9
 Bezzaf li 3ayech 7elma w ga3 ma baghi yfi9 y3i9
 Kangol f rassi nsitek kayfekkerni fik tri9
 W ba9i 3a9el 3la ga3 lkhayeb w ga3 zwin w 3a9el
 Ba9i 3a9el ch7al men merra te7t w l9itek rajel
 L7yat ghi battle w lkebda klitha, la ma b9itch fragile
 Dima l rassi w kif l3ada gha kanzido rassid
 
 [Refrain : Drizzy w Anys]
 Ih, ya lali ya li ya lilali
 Ya khsara, 7ta li bghitha tel3etli chitana
 Ih, ya lali ya li ya lilali
 Ma tgoulch khsara, ga3 li ghderna rah qerrana
 Ih, ya lali ya li ya lilali
 Ya khsara, 7ta li bghitha tel3etli chitana
 Ih, ya lali ya li ya lilali
 Ma tgoulch khsara, ga3 li ghderna rah qerrana