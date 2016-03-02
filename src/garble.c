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
#include "garble/aes.h"
#include "garble_gate_halfgates.h"
#include "garble_gate_standard.h"

#include <assert.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

static inline void
hash2(block *A, block *B, const block tweak, const AES_KEY *key)
{
    block masks[2], keys[2];

    keys[0] = garble_xor(garble_double(*A), tweak);
    keys[1] = garble_xor(garble_double(*B), tweak);
    memcpy(masks, keys, sizeof keys);
    AES_ecb_encrypt_blks(keys, 2, key);
    *A = garble_xor(keys[0], masks[0]);
    *B = garble_xor(keys[1], masks[1]);
}

static void
_garble_privacy_free(garble_circuit *gc, const AES_KEY *key, block delta)
{
    for (uint64_t i = 0; i < gc->q; ++i) {
        garble_gate *g;
        block A0, A1, B0, B1;

        g = &gc->gates[i];
        A0 = gc->wires[g->input0].label0;
        A1 = gc->wires[g->input0].label1;
		B0 = gc->wires[g->input1].label0;
        B1 = gc->wires[g->input1].label1;

        if (g->type == GARBLE_GATE_XOR) {
            gc->wires[g->output].label0 = garble_xor(A0, B0);
            gc->wires[g->output].label1 =
                garble_xor(gc->wires[g->output].label0, delta);
        } else if (g->type == GARBLE_GATE_NOT) {
            assert(0);
        } else {
            block tweak, tmp;

            tweak = garble_make_block(2 * i, (long) 0);
            hash2(&A0, &A1, tweak, key);
            *((char *) &A0) &= 0xfe;
            *((char *) &A1) |= 0x01;
            tmp = garble_xor(A0, A1);
            switch (g->type) {
            case GARBLE_GATE_AND:
                gc->table[i] = garble_xor(tmp, B0);
                gc->wires[g->output].label0 = A0;
                gc->wires[g->output].label1 = garble_xor(A0, delta);
                break;
            case GARBLE_GATE_OR:
                gc->table[i] = garble_xor(tmp, B1);
                gc->wires[g->output].label1 = A1;
                gc->wires[g->output].label0 = garble_xor(A1, delta);
                break;
            default:
                assert(false && "unknown gate type");
                abort();
            }
        }
    }
}


static void
_garble_halfgates(garble_circuit *gc, const AES_KEY *key, block delta)
{
	for (uint64_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];

        garble_gate_halfgates(g,
                              gc->wires[g->input0].label0,
                              gc->wires[g->input0].label1,
                              gc->wires[g->input1].label0,
                              gc->wires[g->input1].label1,
                              &gc->wires[g->output].label0,
                              &gc->wires[g->output].label1,
                              delta, &gc->table[2 * i], i, key);
	}
}

static void
_garble_standard(garble_circuit *gc, const AES_KEY *key, block delta)
{
    for (uint64_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];

        garble_gate_standard(g,
                             gc->wires[g->input0].label0,
                             gc->wires[g->input0].label1,
                             gc->wires[g->input1].label0,
                             gc->wires[g->input1].label1,
                             &gc->wires[g->output].label0,
                             &gc->wires[g->output].label1,
                             delta, &gc->table[3 * i], i, key);
    }
}

int
garble_garble(garble_circuit *gc, const block *inputs, block *outputs)
{
    AES_KEY key;
    block delta, other;

    if (gc == NULL)
        return GARBLE_ERR;

    if (gc->wires == NULL)
        gc->wires = calloc(gc->r, sizeof(garble_wire));

    if (inputs) {
        for (uint64_t i = 0; i < gc->n; ++i) {
            gc->wires[i].label0 = inputs[2 * i];
            gc->wires[i].label1 = inputs[2 * i + 1];
        }
        delta = garble_xor(gc->wires[0].label0, gc->wires[0].label1);
    } else {
        delta = garble_create_delta();
        for (uint64_t i = 0; i < gc->n; ++i) {
            gc->wires[i].label0 = garble_random_block();
            if (gc->type == GARBLE_TYPE_PRIVACY_FREE) {
                *((char *) &gc->wires[i].label0) &= 0xfe;
            }
            gc->wires[i].label1 = garble_xor(gc->wires[i].label0, delta);
        }
    }

#ifdef DEBUG
    for (uint64_t i = gc->n; i < gc->r; ++i) {
        gc->wires[i].label0 = garble_zero_block();
        gc->wires[i].label1 = garble_zero_block();
    }
#endif

    gc->fixed_label = garble_random_block();
    *((char *) &gc->fixed_label) &= 0xfe;
    other = garble_xor(gc->fixed_label, delta);
    for (uint64_t i = 0; i < gc->n_fixed_wires; ++i) {
        gc->wires[gc->fixed_wires[i].idx].label0 = gc->fixed_label;
        gc->wires[gc->fixed_wires[i].idx].label1 = other;
    }

    gc->global_key = garble_random_block();
    AES_set_encrypt_key(gc->global_key, &key);
    if (gc->table == NULL)
        gc->table = calloc(gc->q, garble_table_size(gc));

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

    if (outputs) {
        for (uint64_t i = 0; i < gc->m; ++i) {
            outputs[2*i] = gc->wires[gc->outputs[i]].label0;
            outputs[2*i+1] = gc->wires[gc->outputs[i]].label1;
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
    (void) SHA1_Update(&c, gc->table, gc->q * garble_table_size(gc));
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
garble_create_input_labels(block *labels, uint64_t n, block *delta)
{
    block delta_;

    delta_ = delta ? *delta : garble_create_delta();
	for (uint64_t i = 0; i < 2 * n; i += 2) {
        labels[i] = garble_random_block();
		labels[i + 1] = garble_xor(labels[i], delta_);
	}
}
