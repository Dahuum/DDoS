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
    
    if (string)
        *dp = (int8)0;
    
    return;
}

internal int16 stringlen(int8 *s) {
    int16 n;
    int8 *p;
   
    for (p=s, n=0; *p; p++, n++);
    
    return n;
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

internal void tolowercase(int8 *name) {
    int8* p;
    int16 n;
    
    for (p=name, n=11; n; p++,n--)
        if (*p)
            *p = low(*p);
    return;
}

internal int8 low(int8 c) {
    if ((c > 0x40) && (c < 0x5b))
        return (c + 32);
    return c;
}

internal int8 *findchar(int8 *haystack, int8 needle, bool fromleft) {
    int8 *p;
    int16 idx;
    
    if (!haystack)
        return $1 0;
    
    idx = (stringlen(haystack)-1);
    
    for (p=(fromleft)?haystack:haystack+idx; *p && idx; (fromleft)?p++:p--,idx--)
        if (*p == needle)
            break ;
    
    return (*p == needle) ? 
            p :
        $1 0;
}