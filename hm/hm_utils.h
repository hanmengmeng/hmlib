#ifndef HM_UTILS_H_H
#define HM_UTILS_H_H
#include <stdint.h>
uint32_t htonf(float f)
{
    uint32_t p;
    uint32_t sign;
    if (f < 0) { sign = 1; f = -f; }
    else { sign = 0; }

    p = ((((uint32_t)f)&0x7fff)<<16) | (sign<<31); // whole part and sign
    p |= (uint32_t)(((f - (int)f) * 65536.0f))&0xffff; // fraction
    return p;
}

float ntohf(uint32_t p)
{
    float f = ((p>>16)&0x7fff); // whole part
    f += (p&0xffff) / 65536.0f; // fraction
    if (((p>>31)&0x1) == 0x1) { f = -f; } // sign bit set
    return f;
}

#endif