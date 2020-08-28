// SPDX-License-Identifier: MIT
/**
 * @file  esz_hash.c
 * @brief eszFW hash table and hash generator
 */

#include <stdint.h>

/* djb2 by Dan Bernstein
 * http://www.cse.yorku.ca/~oz/hash.html
 */
uint64_t generate_hash(const unsigned char* name)
{
    uint64_t hash = 5381;
    uint32_t c;

    while ((c = *name++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}
