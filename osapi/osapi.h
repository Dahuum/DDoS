/* osapi.h */
#pragma once

#include "os.h"
#include <sys/stat.h> /* for fstat(); */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define getposixfd(x) fds[x]
#define asserted_initialized() if (!initialized) reterr(ErrInit)

private bool isopen(fd);
private void setupfds();
private void zero(int8*,int16);