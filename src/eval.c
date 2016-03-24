#include "garble.h"
#include "garble/garble_gate_halfgates.h"
#include "garble/garble_gate_privacy_free.h"
#include "garble/garble_gate_standard.h"

#include <assert.h>
#include <string.h>

static inline void
hash1(block *A, block tweak, const AES_KEY *K)
{
    block key;
    block mask;

    key = garble_xor(garble_double(*A), tweak);
    mask = key;
    AES_ecb_encrypt_blks(&key, 1, K);
    *A = garble_xor(key, mask);
}

static void
_eval_privacy_free(const garble_circuit *gc, block *labels, const AES_KEY *key)
{
    for (uint64_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];

        garble_gate_eval_privacy_free(g->type,
                                      labels[g->input0],
                                      labels[g->input1],
                                      &labels[g->output],
                                      &gc->table[i],
                                      i, key);
    }
}

static void
_eval_halfgates(const garble_circuit *gc, block *labels, const AES_KEY *key)
{
    for (uint64_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];

        garble_gate_eval_halfgates(g->type,
                                   labels[g->input0],
                                   labels[g->input1],
                                   &labels[g->output],
                                   &gc->table[2 * i],
                                   i, key);
    }
}

static void
_eval_standard(const garble_circuit *gc, block *labels, AES_KEY *key)
{
    for (uint64_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];

        garble_gate_eval_standard(g->type,
                                  labels[g->input0],
                                  labels[g->input1],
                                  &labels[g->output],
                                  &gc->table[3 * i],
                                  i, key);
    }
}

int
garble_eval(const garble_circuit *gc, const block *input_labels,
            block *output_labels, bool *outputs)
{
    AES_KEY key;
    block *labels;
    block fixed_label;

    if (gc == NULL)
        return GARBLE_ERR;

    AES_set_encrypt_key(gc->global_key, &key);
    labels = garble_allocate_blocks(gc->r);

    /* Set input wire labels */
    memcpy(labels, input_labels, gc->n * sizeof input_labels[0]);

    /* Set fixed wire labels */
    fixed_label = gc->fixed_label;
    for (uint64_t i = 0; i < gc->n_fixed_wires; ++i) {
        switch (gc->fixed_wires[i].type) {
        case GARBLE_FIXED_WIRE_ZERO:
            *((char *) &fixed_label) &= 0xfe;
            labels[gc->fixed_wires[i].idx] = fixed_label;
            break;
        case GARBLE_FIXED_WIRE_ONE:
            *((char *) &fixed_label) |= 0x01;
            labels[gc->fixed_wires[i].idx] = fixed_label;
            break;
        }
    }

    switch (gc->type) {
    case GARBLE_TYPE_STANDARD:
        _eval_standard(gc, labels, &key);
        break;
    case GARBLE_TYPE_HALFGATES:
        _eval_halfgates(gc, labels, &key);
        break;
    case GARBLE_TYPE_PRIVACY_FREE:
        _eval_privacy_free(gc, labels, &key);
        break;
    }


    if (output_labels) {
        for (uint64_t i = 0; i < gc->m; ++i) {
            output_labels[i] = labels[gc->outputs[i]];
        }
    }
    if (outputs) {
        for (uint64_t i = 0; i < gc->m; ++i) {
            outputs[i] =
                (*((char *) &labels[gc->outputs[i]]) & 0x1) ^ gc->output_perms[i];
        }
    }

    free(labels);

    return GARBLE_OK;
}

void
garble_extract_labels(block *extracted_labels, const block *labels,
                      const bool *bits, uint64_t n)
{
    for (uint64_t i = 0; i < n; ++i) {
        extracted_labels[i] = labels[2 * i + (bits[i] ? 1 : 0)];
    }
}

int
garble_map_outputs(const block *output_labels, const block *map, bool *vals,
                   uint64_t m)
{
    for (uint64_t i = 0; i < m; i++) {
        if (garble_equal(map[i], output_labels[2 * i])) {
            vals[i] = 0;
        } else if (garble_equal(map[i], output_labels[2 * i + 1])) {
            vals[i] = 1;
        } else {
            return GARBLE_ERR;
        }
    }
    return GARBLE_OK;
}
