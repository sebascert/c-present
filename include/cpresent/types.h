#ifndef CPRESENT_TYPES_H
#define CPRESENT_TYPES_H

typedef unsigned long long block_t;
typedef struct {
    block_t hi;
    block_t lo;
} key_t;

typedef unsigned char nibble_t;
typedef unsigned char byte_t;

#endif /* CPRESENT_TYPES_H */
