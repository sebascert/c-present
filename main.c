#include "cpresent/cpresent.h"

#include <argp.h>
#include <stdbool.h>
#include <stdlib.h>

static char doc[] =
    "PRESENT implementation: read from a file/stdin and encrypt/decrypt to a "
    "file/stdout.";
static char args_doc[] = "<KEYFILE>";

enum mode_e { MODE_UNSET = 0, MODE_ENCRYPT, MODE_DECRYPT };

static struct argp_option options[] = {
    {"encrypt", 'e', 0, 0,
     "Encrypt input (default if neither -e nor -d is given)", 0},
    {"decrypt", 'd', 0, 0, "Decrypt input", 0},
    {"input", 'i', "FILE", 0, "Read input from FILE instead of stdin", 0},
    {"output", 'o', "FILE", 0, "Write output to FILE instead of stdout", 0},
    {0}};

struct arguments {
    char *key_file;
    char *input_file;
    char *output_file;
    enum mode_e mode;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;
    switch (key) {
        case 'i':
            arguments->input_file = arg;
            break;
        case 'o':
            arguments->output_file = arg;
            break;
        case 'e':
            arguments->mode = MODE_ENCRYPT;
            break;
        case 'd':
            arguments->mode = MODE_DECRYPT;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num == 0) {
                arguments->key_file = arg;
            } else {
                argp_usage(state);
            }
            break;
        case ARGP_KEY_END:
            if (arguments->mode == MODE_UNSET)
                arguments->mode = MODE_ENCRYPT;
            if (!arguments->key_file) {
                argp_error(state, "missing required KEYFILE");
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

key_t key;
FILE *in, *out;

void cleanup()
{
    clean_key(&key);
    if (in && in != stdin)
        fclose(in);
    if (out && out != stdout)
        fclose(out);
}

bool read_key(FILE *kf)
{
    unsigned char kb[10];
    size_t kn = fread(kb, 1, sizeof(kb), kf);
    fclose(kf);

    if (kn != sizeof(kb)) {
        perror("ERROR - key must be exactly 80 bits (10 bytes)");
        return false;
    }

    key.hi = ((unsigned long long)kb[0] << 8) | (unsigned long long)kb[1];
    key.lo =
        ((unsigned long long)kb[2] << 56) | ((unsigned long long)kb[3] << 48) |
        ((unsigned long long)kb[4] << 40) | ((unsigned long long)kb[5] << 32) |
        ((unsigned long long)kb[6] << 24) | ((unsigned long long)kb[7] << 16) |
        ((unsigned long long)kb[8] << 8) | ((unsigned long long)kb[9]);

    return true;
}

int main(int argc, char *argv[])
{
    struct arguments arguments = {.key_file = NULL,
                                  .input_file = NULL,
                                  .output_file = NULL,
                                  .mode = MODE_UNSET};

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    FILE *kf = fopen(arguments.key_file, "rb");
    if (!kf) {
        perror("ERROR - fopen key");
        return EXIT_FAILURE;
    }
    if (!read_key(kf)) {
        return EXIT_FAILURE;
    }

    in = stdin;
    if (arguments.input_file) {
        in = fopen(arguments.input_file, "rb");
        if (!in) {
            perror("ERROR - fopen in");
            goto ERR;
        }
    }

    out = stdout;
    if (arguments.output_file) {
        out = fopen(arguments.output_file, "wb");
        if (!out) {
            perror("ERROR - fopen out");
            goto ERR;
        }
    }

    set_key(key);

    while (1) {
        block_t word = 0, out_word;
        size_t n = fread(&word, 1, sizeof(word), in);
        if (n == 0) {
            if (feof(in))
                break;
            perror("ERROR - fread");
            goto ERR;
        }

        if (arguments.mode == MODE_DECRYPT)
            out_word = decrypt_block(word);
        else
            out_word = encrypt_block(word);

        if (fwrite(&out_word, 1, sizeof(out_word), out) != sizeof(out_word)) {
            perror("ERROR - fwrite");
            goto ERR;
        }
    }

    cleanup();
    return EXIT_SUCCESS;

ERR:
    cleanup();
    return EXIT_FAILURE;
}
