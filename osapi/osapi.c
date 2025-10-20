/* osapi.c */
#define Library
#include "os.h"
#include "osapi.h"

private fd fds[256];

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

public bool laod(fd file, int8 c) {
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

private void zero(int8 *str, int16 size) {
    int8 *p;
    int16 n;
    
    for (n=size, p=str; n; n--, p++) *p = (int8)0;
}

public void init() {
    errnumber = (int8)0;
    setupfds();
    initialized = true;
    
    return;
}