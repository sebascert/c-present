#ifndef CPRESENT_H
#define CPRESENT_H

typedef unsigned long long block_t;
typedef struct {
    block_t hi;
    block_t lo;
} key_t;

typedef unsigned char nibble_t;

#define BLOCK_BITS sizeof(block_t) * 8
#define NIBBLE_SIZE 4

#define STABLE_SIZE 0xF + 1
#define PTABLE_SIZE BLOCK_BITS

#define ROUND_KEYS 32
#define NIBBLES_IN_BLOCK sizeof(block_t) * 8 / NIBBLE_SIZE

void gen_round_keys80(key_t key);
void clean_key(key_t* key);

block_t encrypt_block(block_t block);
block_t decrypt_block(block_t block);

#endif /* CPRESENT_H */
