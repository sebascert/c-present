#include "cpresent/cpresent.h"

#include <memory.h>
#include <stdbool.h>
#include <stddef.h>

static const nibble_t STABLE[STABLE_SIZE] = {
    [0] = 0xC, [1] = 5,   [2] = 6,   [3] = 0xB, [4] = 9,     [5] = 0,
    [6] = 0xA, [7] = 0xD, [8] = 3,   [9] = 0xE, [0xA] = 0xF, [0xB] = 8,
    [0xC] = 4, [0xD] = 7, [0xE] = 1, [0xF] = 2,
};

static const nibble_t INV_STABLE[STABLE_SIZE] = {
    [0] = 5,   [1] = 0xE, [2] = 0xF, [3] = 8,     [4] = 0xC, [5] = 1,
    [6] = 2,   [7] = 0xD, [8] = 0xB, [9] = 4,     [0xA] = 6, [0xB] = 3,
    [0xC] = 0, [0xD] = 7, [0xE] = 9, [0xF] = 0xA,
};

static const size_t INV_PTABLE[PTABLE_SIZE] = {
    0, 4, 8,  12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
    1, 5, 9,  13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61,
    2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62,
    3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63};

static block_t round_keys[ROUND_KEYS];

const block_t AB = 0xFFFFFFFFFFFFFFFFLL;
const block_t H48 = 0xFFFFFFFFFFFF0000LL;
const block_t L16 = 0x000000000000FFFFLL;
const block_t L4 = 0x000000000000000FLL;

block_t sbox_layer(block_t block, bool inv);
block_t pbox_layer(block_t block, bool inv);

void gen_round_keys80(key_t key)
{
    key_t new_key;
    for (block_t i = 0; i < ROUND_KEYS; i++) {
        // last 64 bits
        round_keys[i] = (key.hi & L16) | (key.lo & H48);

        // left round shift by 61
        new_key.hi = (key.lo >> 3) & L16;
        new_key.lo = key.lo << 61;
        new_key.lo |= (key.hi & L16) << 45;
        new_key.lo |= key.lo >> 19;

        // S-Box last 4 bits
        new_key.hi = (new_key.hi & ~L4) | STABLE[new_key.hi >> 12];

        // counter xored with bits 19-15
        new_key.lo ^= i << 14;

        key = new_key;
    }
}

void clean_key(key_t* key)
{
    memset(key, 0, sizeof(key_t));
    memset(round_keys, 0, ROUND_KEYS * sizeof(block_t));
}

block_t encrypt_block(block_t block)
{
    for (size_t i = 0; i < ROUND_KEYS - 1; i++) {
        // state xored with round key
        block ^= round_keys[i];

        block = sbox_layer(block, false);
        block = pbox_layer(block, false);
    }
    block ^= round_keys[ROUND_KEYS - 1];
    return block;
}

block_t decrypt_block(block_t block)
{
    block ^= round_keys[ROUND_KEYS - 1];
    for (size_t i = ROUND_KEYS - 2; i-- > 0;) {
        block = pbox_layer(block, true);
        block = sbox_layer(block, true);

        // revert state xored with round key
        block ^= round_keys[i];
    }
    return block;
}

block_t sbox_layer(block_t block, bool inv)
{
    const nibble_t* st = inv ? INV_STABLE : STABLE;
    for (size_t j = 0; j < NIBBLES_IN_BLOCK; j++) {
        size_t s = 4 * j;
        block = (block & ~(L4 << s)) | st[(block >> s) & L4] << s;
    }
    return block;
}

block_t pbox_layer(block_t block, bool inv)
{
    block_t pblock = 0;
    size_t perms = BLOCK_BITS - 1;
    for (size_t j = 0; j < perms; j++) {
        size_t p = inv ? j * 16 % perms : INV_PTABLE[j];
        pblock = (pblock & ~(1 << p)) | (block & (1 << p));
    }
    return pblock;
}
