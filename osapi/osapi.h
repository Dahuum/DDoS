/* osapi.h */
#pragma once

#include "os.h"
#include <sys/stat.h> /* for fstat(); */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define getposixfd(x) fds[x]
#define asserted_initialized() if (!initialized) reterr(ErrInit)

#ifdef Library
    private bool isopen(fd);
    private void setupfds();
    private int16 stringlen(int8*);
#endif
internal int8 *strnum(int8*, int8);
internal void zero(int8*,int16);
internal void copy(int8*, int8*, int16);

public void dinit(void);