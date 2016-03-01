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

#include <assert.h>
#include <malloc.h>

#include "garble.h"
#include "garble/aes.h"

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

static inline void
hash2(block *A, block *B, block tweak1, block tweak2,
      const AES_KEY *key)
{
    block keys[2];
    block masks[2];

    keys[0] = garble_xor(garble_double(*A), tweak1);
    keys[1] = garble_xor(garble_double(*B), tweak2);
    masks[0] = keys[0];
    masks[1] = keys[1];
    AES_ecb_encrypt_blks(keys, 2, key);
    *A = garble_xor(keys[0], masks[0]);
    *B = garble_xor(keys[1], masks[1]);
}

static void
_eval_halfgates(const garble_circuit *gc, block *labels, const AES_KEY *K)
{
	for (uint64_t i = 0; i < gc->q; i++) {
		garble_gate *g = &gc->gates[i];
		if (g->type == GARBLE_GATE_XOR) {
			labels[g->output] =
                garble_xor(labels[g->input0], labels[g->input1]);
		} else if (g->type == GARBLE_GATE_NOT) {
            block A, tweak;
            unsigned short pa;

            A = labels[g->input0];
            tweak = garble_make_block(2 * i, (long) 0);
            pa = garble_lsb(A);
            hash1(&A, tweak, K);
            labels[g->output] = garble_xor(A, gc->table[2 * i + pa]);
        } else {
            block A, B, W;
            int sa, sb;
            block tweak1, tweak2;

            A = labels[g->input0];
            B = labels[g->input1];

            sa = garble_lsb(A);
            sb = garble_lsb(B);

            tweak1 = garble_make_block(2 * i, (long) 0);
            tweak2 = garble_make_block(2 * i + 1, (long) 0);

            hash2(&A, &B, tweak1, tweak2, K);

            W = garble_xor(A, B);
            if (sa)
                W = garble_xor(W, gc->table[2 * i]);
            if (sb) {
                W = garble_xor(W, gc->table[2 * i + 1]);
                W = garble_xor(W, labels[g->input0]);
            }
            labels[g->output] = W;
        }
	}
}

static void
_eval_standard(const garble_circuit *gc, block *labels, AES_KEY *key)
{
	for (uint64_t i = 0; i < gc->q; i++) {
		garble_gate *g = &gc->gates[i];
		if (g->type == GARBLE_GATE_XOR) {
			labels[g->output] = garble_xor(labels[g->input0], labels[g->input1]);
		} else {
            block A, B, tmp, tweak, val;
            int a, b;

            A = garble_double(labels[g->input0]);
            B = garble_double(garble_double(labels[g->input1]));

            a = garble_lsb(labels[g->input0]);
            b = garble_lsb(labels[g->input1]);

            tweak = garble_make_block(i, (long) 0);
            val = garble_xor(garble_xor(A, B), tweak);
            tmp = a + b ? garble_xor(gc->table[3 * i + 2*a+b-1], val) : val;
            AES_ecb_encrypt_blks(&val, 1, key);

            labels[g->output] = garble_xor(val, tmp);
        }
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
	for (uint64_t i = 0; i < gc->n; ++i) {
        labels[i] = inputs[i];
	}

    /* Set fixed wire labels */
    for (uint64_t i = 0; i < gc->n_fixed_wires; ++i) {
        labels[gc->fixed_wires[i].idx] = gc->fixed_label;
    }

    /* Process gates */
    switch (gc->type) {
    case GARBLE_TYPE_STANDARD:
        _eval_standard(gc, labels, &key);
        break;
    case GARBLE_TYPE_HALFGATES:
        _eval_halfgates(gc, labels, &key);
        break;
    default:
        assert(0);
        abort();
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
