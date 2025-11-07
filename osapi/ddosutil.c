#include "osapi.h" 
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fs.h"
#include <stdio.h>

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
   dd = dattach(drive);
   if (!dd) {
       fprintf(stderr, "Bad disk\n"); 
       exit(-1);
   }
   fs = fsformat(dd, (bootsector *)0, bforce);
   if (fs)
        fsshow(fs, false);
    else 
        fprintf(stderr, "Formatting failed\n");
   
    return ;
}

void usage(char *arg) {
    fprintf(stderr, "Usage: %s <command> [arguments] \nAvailable Commands: format\n", arg);
    exit(-1);
}

void usage_format(char *arg) {
    fprintf(stderr, "Usage: %s [-s] <drive>\nExample: %s format c:\n", arg, arg);
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