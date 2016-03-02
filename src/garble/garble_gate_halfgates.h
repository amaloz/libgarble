#ifndef LIBGARBLE_GARBLE_GATE_HALFGATES_H
#define LIBGARBLE_GARBLE_GATE_HALFGATES_H

#include "garble.h"
#include "garble/aes.h"

#include <assert.h>
#include <string.h>

inline void
garble_gate_halfgates(garble_gate_type_e type, block A0, block A1, block B0,
                      block B1, block *out0, block *out1, block delta,
                      block *table, uint64_t idx, const AES_KEY *key)
{
    if (type == GARBLE_GATE_XOR) {
        *out0 = garble_xor(A0, B0);
        *out1 = garble_xor(*out0, delta);
    } else if (type == GARBLE_GATE_NOT) {
        *out0 = garble_xor(A0, delta);
        *out1 = garble_xor(*out0, delta);
    } else {
        long pa = garble_lsb(A0);
        long pb = garble_lsb(B0);
        block tweak1, tweak2;
        block HA0, HA1, HB0, HB1;
        block tmp, W0;

        tweak1 = garble_make_block(2 * idx, (uint64_t) 0);
        tweak2 = garble_make_block(2 * idx + 1, (uint64_t) 0);

        {
            block masks[4], keys[4];

            keys[0] = garble_xor(garble_double(A0), tweak1);
            keys[1] = garble_xor(garble_double(A1), tweak1);
            keys[2] = garble_xor(garble_double(B0), tweak2);
            keys[3] = garble_xor(garble_double(B1), tweak2);
            memcpy(masks, keys, sizeof keys);
            AES_ecb_encrypt_blks(keys, 4, key);
            HA0 = garble_xor(keys[0], masks[0]);
            HA1 = garble_xor(keys[1], masks[1]);
            HB0 = garble_xor(keys[2], masks[2]);
            HB1 = garble_xor(keys[3], masks[3]);
        }
        switch (type) {
        case GARBLE_GATE_AND:
            table[0] = garble_xor(HA0, HA1);
            if (pb)
                table[0] = garble_xor(table[0], delta);
            W0 = HA0;
            if (pa)
                W0 = garble_xor(W0, table[0]);
            tmp = garble_xor(HB0, HB1);
            table[1] = garble_xor(tmp, A0);
            W0 = garble_xor(W0, HB0);
            if (pb)
                W0 = garble_xor(W0, tmp);
            goto finish;
        case GARBLE_GATE_OR:
            table[0] = garble_xor(HA0, HA1);
            if (!pb)
                table[0] = garble_xor(table[0], delta);
            W0 = pa ? HA1 : HA0;
            if ((!pa * !pb) ^ 1)
                W0 = garble_xor(W0, delta);
            table[1] = garble_xor(HB0, HB1);
            table[1] = garble_xor(table[1], A1);
            W0 = garble_xor(W0, pb ? HB1 : HB0);
            goto finish;
        default:
            assert(0);
            abort();
        }
    finish:
        *out0 = W0;
        *out1 = garble_xor(*out0, delta);
    }
}

#endif
