#include "garble/block.h"
#include "garble/aes.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <openssl/rand.h>

static AES_KEY rand_aes_key;
static uint64_t current_rand_index;

block
garble_seed(block *seed)
{
    block cur_seed;
    current_rand_index = 0;
    if (seed) {
        cur_seed = *seed;
    } else {
        fprintf(stderr, "** insecure seeding of randomness!\n");
        srand(time(NULL));
        cur_seed = _mm_set_epi32(rand(), rand(), rand(), rand());
    }
    AES_set_encrypt_key(cur_seed, &rand_aes_key);
    return cur_seed;
}

#define AES(out, sched)                                          \
    {                                                                   \
        int jx;                                                         \
        out = _mm_xor_si128(out, sched[0]);                             \
        for (jx = 1; jx < 10; jx++)                                     \
            out = _mm_aesenc_si128(out, sched[jx]);                     \
        out = _mm_aesenclast_si128(out, sched[jx]);                     \
    }

inline block
garble_random_block(void)
{
    block out = garble_zero_block();
    ((uint64_t *) &out)[0] = current_rand_index++;
    AES(out, rand_aes_key.rd_key);
    return out;
}

block *
garble_allocate_blocks(size_t nblocks)
{
    block *blks = NULL;;
    (void) posix_memalign((void **) &blks, 128, sizeof(block) * nblocks);
    if (blks == NULL) {
        perror("allocate_blocks");
        return NULL;
    } else {
        return blks;
    }
}

/* void */
/* print_block(FILE *stream, block blk) */
/* { */
/*     uint64_t *val = (uint64_t *) &blk; */
/*     fprintf(stream, "%016lx%016lx", val[1], val[0]); */
/* } */
