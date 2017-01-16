#include "garble.h"
#include "garble/garble_gate_halfgates.h"
#include "garble/garble_gate_privacy_free.h"
#include "garble/garble_gate_standard.h"

#include <assert.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

static void
_garble_privacy_free(garble_circuit *restrict gc, const AES_KEY *restrict key, block delta)
{
    size_t nxors = 0;
    for (uint64_t i = 0; i < gc->q; ++i) {
        garble_gate *g = &gc->gates[i];
        nxors += (g->type == GARBLE_GATE_XOR) ? 1 : 0;
        garble_gate_garble_privacy_free(g->type,
                                        gc->wires[2 * g->input0],
                                        gc->wires[2 * g->input0 + 1],
                                        gc->wires[2 * g->input1],
                                        gc->wires[2 * g->input1 + 1],
                                        &gc->wires[2 * g->output],
                                        &gc->wires[2 * g->output + 1],
                                        delta, &gc->table[i - nxors], i, key);
    }
}


static void
_garble_halfgates(garble_circuit *restrict gc, const AES_KEY *restrict key, block delta)
{
    size_t nxors = 0;
    for (size_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];
        bool isxor = g->type == GARBLE_GATE_XOR;
        nxors += isxor ? 1 : 0;
        garble_gate_garble_halfgates(g->type,
                                     gc->wires[2 * g->input0],
                                     gc->wires[2 * g->input0 + 1],
                                     gc->wires[2 * g->input1],
                                     gc->wires[2 * g->input1 + 1],
                                     &gc->wires[2 * g->output],
                                     &gc->wires[2 * g->output + 1],
                                     delta, isxor ? NULL : &gc->table[2 * (i - nxors)], i, key);
    }
}

static void
_garble_standard(garble_circuit *restrict gc, const AES_KEY *restrict key, block delta)
{
    size_t nxors = 0;
    for (size_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];
        nxors += (g->type == GARBLE_GATE_XOR) ? 1 : 0;
        garble_gate_garble_standard(g->type,
                                    gc->wires[2 * g->input0],
                                    gc->wires[2 * g->input0 + 1],
                                    gc->wires[2 * g->input1],
                                    gc->wires[2 * g->input1 + 1],
                                    &gc->wires[2 * g->output],
                                    &gc->wires[2 * g->output + 1],
                                    delta, &gc->table[3 * (i - nxors)], i, key);
    }
}

int
garble_garble(garble_circuit *restrict gc, const block *restrict input_labels,
              block *restrict output_labels)
{
    AES_KEY key;
    block delta;

    if (gc == NULL)
        return GARBLE_ERR;

    if (gc->wires == NULL) {
        gc->wires = calloc(2 * gc->r, sizeof(block));
        if (gc->wires == NULL)
            return GARBLE_ERR;
    }
    if (gc->table == NULL) {
        gc->table = calloc(gc->q - gc->nxors, garble_table_size(gc));
        if (gc->table == NULL)
            return GARBLE_ERR;
    }
    if (gc->output_perms == NULL) {
        gc->output_perms = calloc(gc->m, sizeof(bool));
        if (gc->output_perms == NULL)
            return GARBLE_ERR;
    }

    if (input_labels) {
        for (uint64_t i = 0; i < gc->n; ++i) {
            gc->wires[2 * i] = input_labels[2 * i];
            gc->wires[2 * i + 1] = input_labels[2 * i + 1];
        }
        /* assumes same delta for all 0/1 labels in 'inputs' */
        delta = garble_xor(gc->wires[0], gc->wires[1]);
    } else {
        delta = garble_create_delta();
        for (uint64_t i = 0; i < gc->n; ++i) {
            gc->wires[2 * i] = garble_random_block();
            if (gc->type == GARBLE_TYPE_PRIVACY_FREE) {
                /* zero label should have 0 permutation bit */
                *((char *) &gc->wires[2 * i]) &= 0xfe;
            }
            gc->wires[2 * i + 1] = garble_xor(gc->wires[2 * i], delta);
        }
    }

    {
        block fixed_label = garble_random_block();
        gc->fixed_label = fixed_label;

        *((char *) &fixed_label) &= 0xfe;
        gc->wires[2 * gc->n] = fixed_label;
        gc->wires[2 * gc->n + 1] = garble_xor(fixed_label, delta);
        *((char *) &fixed_label) |= 0x01;
        gc->wires[2 * (gc->n + 1)] = garble_xor(fixed_label, delta);
        gc->wires[2 * (gc->n + 1) + 1] = fixed_label;
    }

    gc->global_key = garble_random_block();
    AES_set_encrypt_key(gc->global_key, &key);

    switch (gc->type) {
    case GARBLE_TYPE_STANDARD:
        _garble_standard(gc, &key, delta);
        break;
    case GARBLE_TYPE_HALFGATES:
        _garble_halfgates(gc, &key, delta);
        break;
    case GARBLE_TYPE_PRIVACY_FREE:
        _garble_privacy_free(gc, &key, delta);
        break;
    }

    for (uint64_t i = 0; i < gc->m; ++i) {
        gc->output_perms[i] = *((char *) &gc->wires[2 * gc->outputs[i]]) & 0x1;
    }

    if (output_labels) {
        for (uint64_t i = 0; i < gc->m; ++i) {
            output_labels[2*i] = gc->wires[2 * gc->outputs[i]];
            output_labels[2*i+1] = gc->wires[2 * gc->outputs[i] + 1];
        }
    }

    return GARBLE_OK;
}

void
garble_hash(const garble_circuit *gc, unsigned char hash[SHA_DIGEST_LENGTH])
{
    SHA_CTX c;
    memset(hash, '\0', SHA_DIGEST_LENGTH);
    (void) SHA1_Init(&c);
    (void) SHA1_Update(&c, gc->table, (gc->q - gc->nxors) * garble_table_size(gc));
    (void) SHA1_Final(hash, &c);
}

int
garble_check(garble_circuit *gc, const unsigned char hash[SHA_DIGEST_LENGTH])
{
    unsigned char newhash[SHA_DIGEST_LENGTH];

    garble_hash(gc, newhash);
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        if (newhash[i] != hash[i]) {
            return GARBLE_ERR;
        }
    }
    return GARBLE_OK;
}

block
garble_create_delta(void)
{
    block delta = garble_random_block();
    *((char *) &delta) |= 1;
    return delta;
}

void
garble_create_input_labels(block *labels, uint64_t n, block *delta,
                           bool privacyfree)
{
    block delta_;

    delta_ = delta ? *delta : garble_create_delta();
    for (uint64_t i = 0; i < 2 * n; i += 2) {
        labels[i] = garble_random_block();
        if (privacyfree)
            *((char *) &labels[i]) &= 0xfe;
        labels[i + 1] = garble_xor(labels[i], delta_);
    }
}
