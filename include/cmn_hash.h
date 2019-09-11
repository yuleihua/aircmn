#ifndef __CMN_HASH_H
#define __CMN_HASH_H

#include "cmn.h"

void hash_murmur3_32(const void *key, int len, uint32_t seed, void *out);


#endif
