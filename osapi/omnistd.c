/* omnistd.c */
#include "omnistd.h"

internal void zero(int8 *str, int16 size) {
    int8 *p;
    int16 n;
    
    for (n=size, p=str; n; n--, p++) *p = (int8)0;
}

internal void memorycopy(int8 *dst, int8 *src, int16 len, bool string) {
    int16 n;
    int8 *dp, *sp;
    
    for (dp=dst, sp=src, n=len; n; dp++, sp++, n--) {
        if ((string) && !(*sp))
            break;
        else 
            *dp = *sp;
    }
    
    return;
}

internal bool getbit(int8* str, int16 idx) {
    int16 blocks;
    int8 mod;
    void *mem;
    int8 *byte;
    bool bit;
    
    blocks = (idx/8);
    mod    = (idx%8);
    
    mem = (void *)str+blocks;
    byte = $1   mem;
    bit = (bool)getbit_(*byte, mod);
    
    return bit;
}

internal void setbit(int8* str, int16 idx, bool val) {
    int16 blocks;
    int8 mod;
    void *mem;
    int8 *byte;
    
    blocks = (idx/8);
    mod    = (idx%8);
    
    mem = (void *)str+blocks;
    byte = $1   mem;
    if (val) 
        *byte = setbit_(*byte, mod);
    else
        *byte = unsetbit_(*byte, mod);
    
    return ;
}