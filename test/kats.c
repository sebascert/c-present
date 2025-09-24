#include "cpresent/cpresent.h"

#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>

#define suite kats

typedef struct {
    key_t key;
    block_t plaintext;
    block_t cypher;
} TC;

#define gk(h, l) {.hi = h, .lo = l}

ParameterizedTestParameters(suite, paper_vectors)
{
    static TC params[] = {
        {gk(0, 0), 0, 0x5579C1387B228445},
        {gk(0xFFFF, 0xFFFFFFFFFFFFFFFF), 0, 0xE72C46C0F5945049},
        {gk(0, 0), 0xFFFFFFFFFFFFFFFF, 0xA112FFC72F68417B},
        {gk(0xFFFF, 0xFFFFFFFFFFFFFFFF), 0xFFFFFFFFFFFFFFFF,
         0x3333DCD3213210D2},
    };
    size_t nb_params = sizeof(params) / sizeof(TC);

    return cr_make_param_array(TC, params, nb_params);
}
ParameterizedTest(TC *param, suite, paper_vectors)
{
    set_key(param->key);
    block_t cypher = encrypt_block(param->plaintext);
    block_t plaintext = decrypt_block(param->cypher);
    cr_assert(eq(ullong, cypher, param->cypher));
    cr_assert(eq(ullong, plaintext, param->plaintext));
}

#undef suite
