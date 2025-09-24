#ifndef CPRESENT_UTILS_H
#define CPRESENT_UTILS_H

#include "cpresent/types.h"

#include <stdio.h>

// reads key from file, signals error if the key is not exactly 80 bits
int read_key(FILE* kf, key_t* key);

// reads block from binary file
int read_block(FILE* in, block_t* block);

// writes block to binary file
int write_block(FILE* out, block_t block);

#endif /* CPRESENT_UTILS_H */
