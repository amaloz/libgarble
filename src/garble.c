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
garbleCircuitHalfGates(GarbledCircuit *gc, const AES_KEY *K, block R)
{
	for (long i = 0; i < gc->q; i++) {
        Gate *g;
        block A0, A1, B0, B1;

        g = &gc->gates[i];

        A0 = gc->wires[g->input0].label0;
		A1 = gc->wires[g->input0].label1;
		B0 = gc->wires[g->input1].label0;
		B1 = gc->wires[g->input1].label1;

        if (g->type == XORGATE) {
			gc->wires[g->output].label0 = garble_xor(A0, B0);
        } else if (g->type == NOTGATE) {
            block tweak;
            long pa = garble_lsb(A0);

            tweak = garble_make_block(2 * i, (long) 0);
            hash2(&A0, &A1, tweak, K);
            gc->garbledTable[i].table[pa] =
                garble_xor(A0, gc->wires[g->input0].label1);
            gc->garbledTable[i].table[!pa] =
                garble_xor(A1, gc->wires[g->input0].label0);
            gc->wires[g->output].label0 = gc->wires[g->input0].label1;
        } else {
            long pa = garble_lsb(A0);
            long pb = garble_lsb(B0);
            block table[2];
            block tweak1, tweak2;
            block tmp, W0;

            tweak1 = garble_make_block(2 * i, (long) 0);
            tweak2 = garble_make_block(2 * i + 1, (long) 0);

            hash4(&A0, &A1, &B0, &B1, tweak1, tweak2, K);
            switch (g->type) {
            case ANDGATE:
                table[0] = garble_xor(A0, A1);
                if (pb)
                    table[0] = garble_xor(table[0], R);
                W0 = A0;
                if (pa)
                    W0 = garble_xor(W0, table[0]);
                tmp = garble_xor(B0, B1);
                table[1] = garble_xor(tmp, gc->wires[g->input0].label0);
                W0 = garble_xor(W0, B0);
                if (pb)
                    W0 = garble_xor(W0, tmp);
                goto finish;
            case ORGATE:
                table[0] = garble_xor(A0, A1);
                if (!pb)
                    table[0] = garble_xor(table[0], R);
                W0 = pa ? A1 : A0;
                if ((!pa * !pb) ^ 1)
                    W0 = garble_xor(W0, R);
                table[1] = garble_xor(B0, B1);
                table[1] = garble_xor(table[1], gc->wires[g->input0].label1);
                W0 = garble_xor(W0, pb ? B1 : B0);
                goto finish;
            default:
                assert(0);
                abort();
            finish:
                gc->garbledTable[i].table[0] = table[0];
                gc->garbledTable[i].table[1] = table[1];
                gc->wires[g->output].label0 = W0;
            }
        }

        gc->wires[g->output].label1 = garble_xor(gc->wires[g->output].label0, R);
	}
}

static void
garbleCircuitStandard(GarbledCircuit *gc, const AES_KEY *key, block delta)
{
    for (long i = 0; i < gc->q; i++) {
        Gate *g = &gc->gates[i];
        GarbledTable *gt = &gc->garbledTable[i];
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
            && g->type != NOTGATE) {
            abort();
        }

        if (g->type == XORGATE) {
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
        case ANDGATE:
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
        case ORGATE:
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
        case NOTGATE:
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
            gt->table[2*lsb0 + lsb1 -1] = garble_xor(blocks[0], mask[0]);
        if (2*lsb0 + 1-lsb1 != 0)
            gt->table[2*lsb0 + 1-lsb1-1] = garble_xor(blocks[1], mask[1]);
        if (2*(1-lsb0) + lsb1 != 0)
            gt->table[2*(1-lsb0) + lsb1-1] = garble_xor(blocks[2], mask[2]);
        if (2*(1-lsb0) + (1-lsb1) != 0)
            gt->table[2*(1-lsb0) + (1-lsb1)-1] = garble_xor(blocks[3], mask[3]);
    }
}

int
garble_garble(GarbledCircuit *gc, const block *inputLabels, block *outputLabels,
              GarbleType type)
{
    AES_KEY key;
    block delta, other;

    if (gc == NULL)
        return GARBLE_ERR;

    if (inputLabels) {
        for (int i = 0; i < gc->n; ++i) {
            gc->wires[i].label0 = inputLabels[2 * i];
            gc->wires[i].label1 = inputLabels[2 * i + 1];
        }
        delta = garble_xor(gc->wires[0].label0, gc->wires[0].label1);
    } else {
        delta = garble_random_block();
        for (int i = 0; i < gc->n; ++i) {
            gc->wires[i].label0 = garble_random_block();
            gc->wires[i].label1 = garble_xor(gc->wires[i].label0, delta);
        }
    }

    for (int i = gc->n; i < gc->r; ++i) {
        gc->wires[i].label0 = garble_zero_block();
        gc->wires[i].label1 = garble_zero_block();
    }

    gc->fixedLabel = garble_random_block();
    other = garble_xor(gc->fixedLabel, delta);
    for (int i = 0; i < gc->nFixedWires; ++i) {
        switch (gc->fixedWires[i].type) {
        case FIXED_WIRE_ZERO:
            gc->wires[gc->fixedWires[i].idx].label0 = gc->fixedLabel;
            gc->wires[gc->fixedWires[i].idx].label1 = other;
            break;
        case FIXED_WIRE_ONE:
            gc->wires[gc->fixedWires[i].idx].label0 = other;
            gc->wires[gc->fixedWires[i].idx].label1 = gc->fixedLabel;
            break;
        }
    }

    gc->globalKey = garble_random_block();
    AES_set_encrypt_key(gc->globalKey, &key);
    switch (type) {
    case GARBLE_TYPE_STANDARD:
        garbleCircuitStandard(gc, &key, delta);
        break;
    case GARBLE_TYPE_HALFGATES:
        garbleCircuitHalfGates(gc, &key, delta);
        break;
    default:
        assert(0);
        abort();
    }

    if (outputLabels) {
        for (int i = 0; i < gc->m; ++i) {
            outputLabels[2*i] = gc->wires[gc->outputs[i]].label0;
            outputLabels[2*i+1] = gc->wires[gc->outputs[i]].label1;
        }
    }

    return GARBLE_OK;
}

void
garble_hash(const GarbledCircuit *gc,
            unsigned char hash[SHA_DIGEST_LENGTH],
            GarbleType type)
{
    memset(hash, '\0', SHA_DIGEST_LENGTH);
    for (int i = 0; i < gc->q; ++i) {
        SHA_CTX c;

        (void) SHA1_Init(&c);
        (void) SHA1_Update(&c, hash, SHA_DIGEST_LENGTH);
        (void) SHA1_Update(&c, &gc->garbledTable[i], sizeof(GarbledTable));
        (void) SHA1_Final(hash, &c);
    }
}

int
garble_check(GarbledCircuit *gc,
             const unsigned char hash[SHA_DIGEST_LENGTH],
             GarbleType type)
{
    unsigned char newhash[SHA_DIGEST_LENGTH];

    garble_hash(gc, newhash, type);
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
garble_create_input_labels(block *labels, int n, block *delta)
{
    block delta_;

    delta_ = delta ? *delta : garble_create_delta();
	for (int i = 0; i < 2 * n; i += 2) {
        labels[i] = garble_random_block();
		labels[i + 1] = garble_xor(labels[i], delta_);
	}
}
