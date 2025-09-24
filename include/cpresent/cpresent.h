#ifndef CPRESENT_H
#define CPRESENT_H

#include "cpresent/types.h"

void set_key(key_t key);
void clean_key(key_t* key);

block_t encrypt_block(block_t block);
block_t decrypt_block(block_t block);

#endif /* CPRESENT_H */
