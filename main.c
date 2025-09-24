#include "cpresent/cpresent.h"
#include "cpresent/utils.h"

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
            if (!arguments->key_file)
                argp_error(state, "missing required KEYFILE");
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

int main(int argc, char *argv[])
{
    struct arguments arguments = {.key_file = NULL,
                                  .input_file = NULL,
                                  .output_file = NULL,
                                  .mode = MODE_UNSET};

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    FILE *kf = fopen(arguments.key_file, "rb");
    if (read_key(kf, &key)) {
        fclose(kf);
        return EXIT_FAILURE;
    }

    in = stdin;
    if (arguments.input_file) {
        in = fopen(arguments.input_file, "rb");
        if (!in) {
            fprintf(stderr, "ERROR - fopen in\n");
            goto ERR;
        }
    }

    out = stdout;
    if (arguments.output_file) {
        out = fopen(arguments.output_file, "wb");
        if (!out) {
            fprintf(stderr, "ERROR - fopen out\n");
            goto ERR;
        }
    }

    set_key(key);

    while (1) {
        block_t word = 0, out_word;
        if (read_block(in, &word)) {
            if (feof(in))
                break;
            fprintf(stderr, "ERROR - fread\n");
            goto ERR;
        }

        if (arguments.mode == MODE_DECRYPT)
            out_word = decrypt_block(word);
        else
            out_word = encrypt_block(word);

        if (write_block(out, out_word)) {
            fprintf(stderr, "ERROR - fwrite\n");
            goto ERR;
        }
    }

    cleanup();
    return EXIT_SUCCESS;

ERR:
    cleanup();
    return EXIT_FAILURE;
}
