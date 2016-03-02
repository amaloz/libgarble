/*
 This file is part of JustGarble.

    JustGarble is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    JustGarble is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.

*/

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
garble_eval(const garble_circuit *gc, const block *inputs, block *outputs)
{
    AES_KEY key;
    block *labels;

    if (gc == NULL)
        return GARBLE_ERR;

	AES_set_encrypt_key(gc->global_key, &key);
    labels = garble_allocate_blocks(gc->r);

    /* Set input wire labels */
    memcpy(labels, inputs, gc->n * sizeof inputs[0]);

    /* Set fixed wire labels */
    for (uint64_t i = 0; i < gc->n_fixed_wires; ++i) {
        switch (gc->fixed_wires[i].type) {
        case GARBLE_FIXED_WIRE_ZERO:
            *((char *) &gc->fixed_label) &= 0xfe;
            labels[gc->fixed_wires[i].idx] = gc->fixed_label;
            break;
        case GARBLE_FIXED_WIRE_ONE:
            *((char *) &gc->fixed_label) |= 0x01;
            labels[gc->fixed_wires[i].idx] = gc->fixed_label;
            break;
        }
    }

    /* Process gates */
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

    /* Set output wire labels */
    if (outputs) {
        for (uint64_t i = 0; i < gc->m; i++) {
            outputs[i] = labels[gc->outputs[i]];
        }
    }

    free(labels);

    return GARBLE_OK;
}

void
garble_extract_labels(block *extracted, const block *labels, const bool *bits,
                      uint64_t n)
{
    for (uint64_t i = 0; i < n; ++i) {
        extracted[i] = labels[2 * i + (bits[i] ? 1 : 0)];
    }
}

int
garble_map_outputs(const block *outputs, const block *map, bool *vals,
                   uint64_t m)
{
	for (uint64_t i = 0; i < m; i++) {
        if (garble_equal(map[i], outputs[2 * i])) {
			vals[i] = 0;
        } else if (garble_equal(map[i], outputs[2 * i + 1])) {
			vals[i] = 1;
		} else {
            printf("MAP OUTPUTS FAILED %ld\n", i);
            return GARBLE_ERR;
        }
	}
	return GARBLE_OK;
}
