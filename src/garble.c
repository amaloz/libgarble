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

static inline void
hash4(block *A0, block *A1, block *B0, block *B1,
      const block tweak1, const block tweak2, const AES_KEY *key)
{
    block masks[4], keys[4];

    keys[0] = garble_xor(garble_double(*A0), tweak1);
    keys[1] = garble_xor(garble_double(*A1), tweak1);
    keys[2] = garble_xor(garble_double(*B0), tweak2);
    keys[3] = garble_xor(garble_double(*B1), tweak2);
    memcpy(masks, keys, sizeof keys);
    AES_ecb_encrypt_blks(keys, 4, key);
    *A0 = garble_xor(keys[0], masks[0]);
    *A1 = garble_xor(keys[1], masks[1]);
    *B0 = garble_xor(keys[2], masks[2]);
    *B1 = garble_xor(keys[3], masks[3]);
}

static void
_garble_halfgates(garble_circuit *gc, const AES_KEY *K, block delta)
{
	for (uint64_t i = 0; i < gc->q; i++) {
        garble_gate *g;
        block A0, A1, B0, B1;

        g = &gc->gates[i];

        A0 = gc->wires[g->input0].label0;
		A1 = gc->wires[g->input0].label1;
		B0 = gc->wires[g->input1].label0;
		B1 = gc->wires[g->input1].label1;

        if (g->type == GARBLE_GATE_XOR) {
			gc->wires[g->output].label0 = garble_xor(A0, B0);
        } else if (g->type == GARBLE_GATE_NOT) {
            block tweak;
            long pa = garble_lsb(A0);

            tweak = garble_make_block(2 * i, (long) 0);
            hash2(&A0, &A1, tweak, K);
            gc->table[2 * i + pa] = garble_xor(A0, gc->wires[g->input0].label1);
            gc->table[2 * i + !pa] = garble_xor(A1, gc->wires[g->input0].label0);
            gc->wires[g->output].label0 = gc->wires[g->input0].label1;
        } else {
            long pa = garble_lsb(A0);
            long pb = garble_lsb(B0);
            block tweak1, tweak2;
            block tmp, W0;

            tweak1 = garble_make_block(2 * i, (long) 0);
            tweak2 = garble_make_block(2 * i + 1, (long) 0);

            hash4(&A0, &A1, &B0, &B1, tweak1, tweak2, K);
            switch (g->type) {
            case GARBLE_GATE_AND:
                gc->table[2 * i] = garble_xor(A0, A1);
                if (pb)
                    gc->table[2 * i] = garble_xor(gc->table[2 * i], delta);
                W0 = A0;
                if (pa)
                    W0 = garble_xor(W0, gc->table[2 * i]);
                tmp = garble_xor(B0, B1);
                gc->table[2 * i + 1] = garble_xor(tmp, gc->wires[g->input0].label0);
                W0 = garble_xor(W0, B0);
                if (pb)
                    W0 = garble_xor(W0, tmp);
                goto finish;
            case GARBLE_GATE_OR:
                gc->table[2 * i] = garble_xor(A0, A1);
                if (!pb)
                    gc->table[2 * i] = garble_xor(gc->table[2 * i], delta);
                W0 = pa ? A1 : A0;
                if ((!pa * !pb) ^ 1)
                    W0 = garble_xor(W0, delta);
                gc->table[2 * i + 1] = garble_xor(B0, B1);
                gc->table[2 * i + 1] = garble_xor(gc->table[2 * i + 1],
                                                  gc->wires[g->input0].label1);
                W0 = garble_xor(W0, pb ? B1 : B0);
                goto finish;
            default:
                assert(0);
                abort();
            finish:
                gc->wires[g->output].label0 = W0;
            }
        }

        gc->wires[g->output].label1 = garble_xor(gc->wires[g->output].label0,
                                                 delta);
	}
}

static void
_garble_standard(garble_circuit *gc, const AES_KEY *key, block delta)
{
    for (uint64_t i = 0; i < gc->q; i++) {
        garble_gate *g = &gc->gates[i];
        int input0, input1, output;
        block tweak, blocks[4], keys[4], mask[4];
        block A0, A1, B0, B1;
        block newToken, newToken2;
        block *label0, *label1;
        long lsb0, lsb1;

        input0 = g->input0;
        input1 = g->input1;
        output = g->output;

        if ((garble_equal(gc->wires[input0].label0, garble_zero_block())
            || garble_equal(gc->wires[input0].label1, garble_zero_block())
            || garble_equal(gc->wires[input1].label0, garble_zero_block())
            || garble_equal(gc->wires[input1].label1, garble_zero_block()))
            && g->type != GARBLE_GATE_NOT) {
            abort();
        }

        if (g->type == GARBLE_GATE_XOR) {
            gc->wires[output].label0 =
                garble_xor(gc->wires[input0].label0, gc->wires[input1].label0);
            gc->wires[output].label1 =
                garble_xor(gc->wires[input0].label1, gc->wires[input1].label0);
            continue;
        }
        tweak = garble_make_block(i, (long)0);
        lsb0 = garble_lsb(gc->wires[input0].label0);
        lsb1 = garble_lsb(gc->wires[input1].label0);

        A0 = garble_double(gc->wires[input0].label0);
        A1 = garble_double(gc->wires[input0].label1);
        B0 = garble_double(garble_double(gc->wires[input1].label0));
        B1 = garble_double(garble_double(gc->wires[input1].label1));

        keys[0] = garble_xor(garble_xor(A0, B0), tweak);
        keys[1] = garble_xor(garble_xor(A0, B1), tweak);
        keys[2] = garble_xor(garble_xor(A1, B0), tweak);
        keys[3] = garble_xor(garble_xor(A1, B1), tweak);
        memcpy(mask, keys, sizeof mask);
        AES_ecb_encrypt_blks(keys, 4, key);
        mask[0] = garble_xor(mask[0], keys[0]);
        mask[1] = garble_xor(mask[1], keys[1]);
        mask[2] = garble_xor(mask[2], keys[2]);
        mask[3] = garble_xor(mask[3], keys[3]);

        newToken = mask[2 * lsb0 + lsb1];
        newToken2 = garble_xor(delta, newToken);
        label0 = &gc->wires[output].label0;
        label1 = &gc->wires[output].label1;

        switch (g->type) {
        case GARBLE_GATE_AND:
            if (lsb1 & lsb0) {
                *label0 = newToken2;
                *label1 = newToken;
            } else {
                *label0 = newToken;
                *label1 = newToken2;
            }
            blocks[0] = *label0;
            blocks[1] = *label0;
            blocks[2] = *label0;
            blocks[3] = *label1;
            break;
        case GARBLE_GATE_OR:
            if (lsb1 | lsb0) {
                *label0 = newToken2;
                *label1 = newToken;
            } else {
                *label0 = newToken;
                *label1 = newToken2;
            }
            blocks[0] = *label0;
            blocks[1] = *label1;
            blocks[2] = *label1;
            blocks[3] = *label1;
            break;
        case GARBLE_GATE_NOT:
            if (!lsb0) {
                *label0 = newToken2;
                *label1 = newToken;
            } else {
                *label0 = newToken;
                *label1 = newToken2;
            }
            blocks[0] = *label1;
            blocks[1] = *label0;
            blocks[2] = *label1;
            blocks[3] = *label0;
            break;
        default:
            assert(0);
            abort();
        }

        if (2*lsb0 + lsb1 != 0)
            gc->table[3 * i + 2*lsb0 + lsb1 -1] = garble_xor(blocks[0], mask[0]);
        if (2*lsb0 + 1-lsb1 != 0)
            gc->table[3 * i + 2*lsb0 + 1-lsb1-1] = garble_xor(blocks[1], mask[1]);
        if (2*(1-lsb0) + lsb1 != 0)
            gc->table[3 * i + 2*(1-lsb0) + lsb1-1] = garble_xor(blocks[2], mask[2]);
        if (2*(1-lsb0) + (1-lsb1) != 0)
            gc->table[3 * i + 2*(1-lsb0) + (1-lsb1)-1] = garble_xor(blocks[3], mask[3]);
    }
}

int
garble_garble(garble_circuit *gc, const block *inputs, block *outputs)
{
    AES_KEY key;
    block delta, other;

    if (gc == NULL)
        return GARBLE_ERR;

    if (inputs) {
        for (uint64_t i = 0; i < gc->n; ++i) {
            gc->wires[i].label0 = inputs[2 * i];
            gc->wires[i].label1 = inputs[2 * i + 1];
        }
        delta = garble_xor(gc->wires[0].label0, gc->wires[0].label1);
    } else {
        delta = garble_random_block();
        for (uint64_t i = 0; i < gc->n; ++i) {
            gc->wires[i].label0 = garble_random_block();
            gc->wires[i].label1 = garble_xor(gc->wires[i].label0, delta);
        }
    }

    for (uint64_t i = gc->n; i < gc->r; ++i) {
        gc->wires[i].label0 = garble_zero_block();
        gc->wires[i].label1 = garble_zero_block();
    }

    gc->fixed_label = garble_random_block();
    other = garble_xor(gc->fixed_label, delta);
    for (uint64_t i = 0; i < gc->n_fixed_wires; ++i) {
        switch (gc->fixed_wires[i].type) {
        case GARBLE_FIXED_WIRE_ZERO:
            gc->wires[gc->fixed_wires[i].idx].label0 = gc->fixed_label;
            gc->wires[gc->fixed_wires[i].idx].label1 = other;
            break;
        case GARBLE_FIXED_WIRE_ONE:
            gc->wires[gc->fixed_wires[i].idx].label0 = other;
            gc->wires[gc->fixed_wires[i].idx].label1 = gc->fixed_label;
            break;
        }
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
    default:
        assert(0);
        abort();
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
