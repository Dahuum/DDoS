/* osapi.h */
#pragma once

#include "os.h"
#include "disk.h"
#include "omnistd.h"
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

internal void closeallfiles(disk* dd);
internal int16 openfiles(disk* dd);


public void init(void);
public void dinit(void);