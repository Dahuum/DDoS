/* omnistd.h */
#pragma once

#include "osapi.h"

#define kprintf(f,args ...) printf(f "\n",args)

#define copy(d,s,l)         memorycopy(d,s,l,false)
#define stringcopy(d,s,l)   memorycopy(d,s,l,true)

#define getbit_(b,p)        ((b & (1 << p)) >> p)
#define unsetbit_(b,p)      (b & ~(1 << p))
#define setbit_(b,p)        (b | (1 << p))

internal void zero(int8*,int16);
internal void memorycopy(int8*, int8*, int16, bool);

internal bool getbit(int8*,int16);
internal void setbit(int8*,int16,bool);