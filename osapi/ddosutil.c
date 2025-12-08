#include "omnistd.h"
#include "os.h"
#include "osapi.h" 
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fs.h"
#include <stdio.h>
#include <sys/stat.h>

void usage(char*);
void usage_format(char*);
int main(int,char**);
void cmd_format(char*,char*);

void cmd_format(char *arg1, char *arg2) {
   bool bootable; int8 drive; int8 *drivestr; char force; bool bforce;
   disk *dd;
   filesystem *fs;
   
   if (!arg1)
       usage_format("diskutil");
   else if (!arg2) {
      bootable = false; 
      drivestr = $1 arg1;
   }
   else {
       if ((*arg2 == '-') && (arg1[1] == 's'))  {
           drivestr = $1 arg2;
       }
       else 
           usage_format("diskutil");
   }
   
   switch (*drivestr) {
        case 'c':
            drive = 1;
            break ;
        case 'd':
            drive = 2;
            break ;
        default:
            usage_format("diskutil");
            break;
   }
   
   if (bootable) {
       fprintf(stderr, "Bootable currently not supported\n");
       exit(-1);
   }
   
   printf("This will format and ERASE  you disk %s\nForce?(Y/N): ", drivestr);
   fflush(stdout);
   
   scanf("%c", &force);
   bforce = ((force == 'y') || (force == 'Y')) ? true : false;
   
   printf("Formatting disk %s\n", drivestr);
   dinit();
   // dd = dattach(drive);
   // if (!dd) {
   //     fprintf(stderr, "Bad disk\n"); 
   //     exit(-1);
   // }
   /* <== // Test Area // ==> */
   ptr idx, idx1;
   path *filepath;
   filename name;
   int8 *fpath, *fpath1, *fpath2;
   int16 size = sizeof(struct s_filename);
   /* (xx) -> fsformat to fsmount */
//    fs = fsmount(drive)/*, (bootsector *)0, bforce)*/; 
   fs = fsformat(DiskDescriptor[drive-1], (bootsector *)0, bforce);
   if (!fs) {
       printf("formatting failed");
       return ;
   }
   inode *root = findinode(fs, $2 0);
   
   zero($1 &name, size);
   copy($1 &name.name, $1 "auto", $2 4);
   copy($1 &name.ext, $1 "cpp", $2 3);
   idx = increate(fs, &name, TypeFile);
   root->direct[0] = idx;
   int16 rett = fssaveinode(fs, root, $2 0);
   
   // printf("idx=%d\n", $i idx);
   
   zero($1 &name, size);
   stringcopy($1 &name.name, $1 "ddos", $2 4);
   idx1 = increate(fs, &name, TypeDir);
   root->direct[1] =  idx1;
   rett = fssaveinode(fs, root, $2 0);
   // printf("idx1=%d\n", $i idx1);
   // return ;
   
    // if (!fs)
        // return; // fsshow(fs, false);
    // else 
        // fprintf(stderr, "Formatting failed\n");
    fpath = $1 strdup("c:/");
    
    directory *dir = (directory *)opendir(fpath);
    if (!dir)
        printf("no opened dir\nerr=0x%.02hhx\n", (char)errnumber);
    if (dir) {
        printf("\033[1m" "Root directory contents: \n" "\033[0m"  );
        for (int n=0; n<dir->len; n++)
            printf("\033[1m" "\033[33m" "  -  '%s'\n" "\033[0m" "\033[0m", file2str(&dir->filelist[n].name));
        destroy(dir);
    }
    
    fpath1 = $1 strdup("c:/ddos/folder////");
    printf("\n" "\033[1m" "\033[33m" "Starting makedir(%s) \n" "\033[0m" "\033[0m" , fpath1);
    idx = makedir(fpath1);
    if (!idx) {
        printf("Error creating '%s', err=0x%.02hhx\n\n", fpath1, (char)errnumber);
        return;
    } else {
        printf("\033[1m" "\033[33m" "Created '%s' with inode %d\n" "\033[0m" "\033[0m" , fpath1, idx);
    }
    fpath2 = $1 strdup("c:/ddos/////");
    dir = opendir(fpath2);
    if (dir) {
        printf("\033[1m" "\033[33m" "'%s' opened successfully (empty dir with %d files)\n" "\033[0m" "\033[0m",fpath2,  dir->len);
        for (int n=0; n<dir->len; n++)
            printf("\033[1m" "\033[33m" "  -  '%s'\n" "\033[0m" "\033[0m", file2str(&dir->filelist[n].name));
        destroy(dir);
    }
    
    destroy(fpath);
    destroy(fpath1);
    destroy(fpath2);
    destroy(root);
    
    // if (idx1) indestroy(fs, idx1);
    // if (fs) fsshow(fs, false);
    /* <== // Test Area // ==> */
   
    return ;
}

void usage(char *arg) {
    fprintf(stderr, "Usage: %s <command> [arguments] \nAvailable Commands: format\n", arg);
    exit(-1);
}

void usage_format(char *arg) {
    fprintf(stderr, "Usage: %s [] <drive>\nExample: %s format c:\n", arg, arg);
    exit(-1);
}

int main(int ac,char **av) {
    char *cmd, *arg1, *arg2;
    
    if (ac < 2)
        usage(*av);
    else if (ac == 2)
        arg1 = arg2 = 0;
    else if (ac == 3) {
        arg1 = av[2];
        arg2 = 0;
    }
    else {
        arg1 = av[2];
        arg2 = av[3];
    }
    cmd = av[1];
    
    if (!strcmp(cmd, "format"))
        cmd_format(arg1, arg2);
    else
        usage(*av);
    
    return 0;
}