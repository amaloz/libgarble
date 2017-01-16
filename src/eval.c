#include "garble.h"
#include "garble/garble_gate_halfgates.h"
#include "garble/garble_gate_privacy_free.h"
#include "garble/garble_gate_standard.h"

#include <assert.h>
#include <string.h>

static void
_eval_privacy_free(const garble_circuit *gc, block *labels, const AES_KEY *key)
{
    size_t nxors = 0;
    for (size_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];
        nxors += (g->type == GARBLE_GATE_XOR ? 1 : 0);
        garble_gate_eval_privacy_free(g->type,
                                      labels[g->input0],
                                      labels[g->input1],
                                      &labels[g->output],
                                      &gc->table[i - nxors],
                                      i, key);
    }
}

static void
_eval_halfgates(const garble_circuit *gc, block *labels, const AES_KEY *key)
{
    size_t nxors = 0;
    for (size_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];
        nxors += (g->type == GARBLE_GATE_XOR ? 1 : 0);
        garble_gate_eval_halfgates(g->type,
                                   labels[g->input0],
                                   labels[g->input1],
                                   &labels[g->output],
                                   &gc->table[2 * (i - nxors)],
                                   i, key);
    }
}

static void
_eval_standard(const garble_circuit *gc, block *labels, AES_KEY *key)
{
    size_t nxors = 0;
    for (size_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];
        nxors += (g->type == GARBLE_GATE_XOR ? 1 : 0);
        garble_gate_eval_standard(g->type,
                                  labels[g->input0],
                                  labels[g->input1],
                                  &labels[g->output],
                                  &gc->table[3 * (i - nxors)],
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
    *((char *) &fixed_label) &= 0xfe;
    labels[gc->n] = fixed_label;
    *((char *) &fixed_label) |= 0x01;
    labels[gc->n + 1] = fixed_label;

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
        for (size_t i = 0; i < gc->m; ++i) {
            output_labels[i] = labels[gc->outputs[i]];
        }
    }
    if (outputs) {
        for (size_t i = 0; i < gc->m; ++i) {
            outputs[i] =
                (*((char *) &labels[gc->outputs[i]]) & 0x1) ^ gc->output_perms[i];
        }
    }

    free(labels);

    return GARBLE_OK;
}

void
garble_extract_labels(block *extracted_labels, const block *labels,
                      const bool *bits, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        extracted_labels[i] = labels[2 * i + (bits[i] ? 1 : 0)];
    }
}

int
garble_map_outputs(const block *output_labels, const block *map, bool *vals,
                   size_t m)
{
    for (size_t i = 0; i < m; i++) {
        if (garble_equal(map[i], output_labels[2 * i])) {
            vals[i] = false;
        } else if (garble_equal(map[i], output_labels[2 * i + 1])) {
            vals[i] = true;
        } else {
            return GARBLE_ERR;
        }
    }
    return GARBLE_OK;
}
