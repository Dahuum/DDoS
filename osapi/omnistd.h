/* omnistd.h */
#pragma once

#include "osapi.h"

#define copy(d,s,l)         memorycopy(d,s,l,false)
#define stringcopy(d,s,l)   memorycopy(d,s,l,true)

internal void zero(int8*,int16);
internal void memorycopy(int8*, int8*, int16, bool);