#include "cpresent/types.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int read_bytes(FILE* in, byte_t* bytes, size_t nmemb, bool pad);

#define KEY_BYTES 10
#define BLOCK_BYTES 8

static inline block_t load64_be(const byte_t b[8])
{
    return ((block_t)b[0] << 56) | ((block_t)b[1] << 48) |
           ((block_t)b[2] << 40) | ((block_t)b[3] << 32) |
           ((block_t)b[4] << 24) | ((block_t)b[5] << 16) |
           ((block_t)b[6] << 8) | ((block_t)b[7]);
}

int read_key(FILE* kf, key_t* key)
{
    byte_t kb[KEY_BYTES];
    if (read_bytes(kf, kb, KEY_BYTES, /*pad=*/false))
        return 1;

    int c = fgetc(kf);
    if (c != EOF) {
        ungetc(c, kf);
        return 1;
    }

    key->hi = ((block_t)kb[0] << 8) | (block_t)kb[1];
    key->lo = load64_be(kb + 2);
    return 0;
}

int read_block(FILE* in, block_t* block)
{
    byte_t b[BLOCK_BYTES];
    if (read_bytes(in, b, BLOCK_BYTES, true))
        return 1;
    *block = load64_be(b);
    return 0;
}

int write_block(FILE* out, block_t block)
{
    byte_t buf[8];
    block_t tmp = block;

    buf[0] = (block_t)(tmp >> 56);
    buf[1] = (block_t)(tmp >> 48);
    buf[2] = (block_t)(tmp >> 40);
    buf[3] = (block_t)(tmp >> 32);
    buf[4] = (block_t)(tmp >> 24);
    buf[5] = (block_t)(tmp >> 16);
    buf[6] = (block_t)(tmp >> 8);
    buf[7] = (block_t)(tmp);

    return fwrite(buf, 1, sizeof buf, out) != sizeof buf;
}

int read_bytes(FILE* in, byte_t* bytes, size_t nmemb, bool pad)
{
    size_t n = fread(bytes, 1, nmemb, in);
    if (n == 0 || (!pad && n < nmemb))
        return 1;
    if (n < nmemb)
        memset(bytes + n, 0, nmemb - n);

#ifndef LITTLE_ENDIAN_ENCODING
    // swap bytes from little-endian to big-endian
    /*for (size_t i = 0; i < sizeof(block_t) / 2; i++) {*/
    /*    byte_t t = bytes[i];*/
    /*    bytes[i] = bytes[sizeof(block_t) - i - 1];*/
    /*    bytes[sizeof(block_t) - i - 1] = t;*/
    /*}*/
#endif /* ifndef LITTLE_ENDIAN_ENCODING */
    return 0;
}
