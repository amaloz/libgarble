#ifndef LIBGARBLEC_CIRCUITS_H
#define LIBGARBLEC_CIRCUITS_H

#include <garble.h>

void
circuit_and(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs);
void
circuit_or(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs);
void
circuit_xor(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs);
void
circuit_not(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs);
void
circuit_mixed(garble_circuit *gc, garble_context *ctxt, uint64_t n,
              const int *inputs, int outputs[1]);
void
circuit_multi_xor(garble_circuit *gc, garble_context *ctxt, int d,
                  uint64_t n, const int *inputs, int *outputs);

void
circuit_mux21(garble_circuit *gc, garble_context *ctxt, int theSwitch,
              int input0, int input1, int output[1]);

/* GF operations */

void
circuit_GF256_inv(garble_circuit *gc, garble_context *ctxt, const int inputs[8],
                  int outputs[8]);
void
circuit_GF16_inv(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
                 int outputs[4]);
void
circuit_GF16_mul(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
                 int outputs[4]);
void
circuit_GF16_sqcl(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
                  int outputs[4]);
void
circuit_GF8_mul(garble_circuit *gc, garble_context *ctxt, const int inputs[8],
                int outputs[8]);
void
circuit_GF4_mul(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
                int outputs[2]);
void
circuit_GF4_scln(garble_circuit *gc, garble_context *ctxt, const int inputs[2],
                 int outputs[2]);
void
circuit_GF4_scln2(garble_circuit *gc, garble_context *ctxt, const int inputs[2],
                  int outputs[2]);
void
circuit_GF4_sq(const int inputs[2], int outputs[2]);

void
circuit_rand(garble_circuit *gc, garble_context *ctxt, uint64_t n,
             int outputs[1], int q, int qf);

void
circuit_inc(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs);
void
circuit_sub(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs);

void
circuit_shl(garble_circuit *gc, uint64_t n, const int *inputs, int *outputs);
void
circuit_shr(garble_circuit *gc, uint64_t n, const int *inputs, int *outputs);
void
circuit_mul(garble_circuit *gc, garble_context *ctxt, uint64_t nt,
            int *inputs, int *outputs);
void
circuit_gre(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs);
void
circuit_min(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            int *inputs, int *outputs);
void
circuit_leq(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs);
void
circuit_geq(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs);
void
circuit_les(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs);
void
circuit_equ(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int outputs[1]);
void
circuit_add(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs, int *carry);
void
circuit_add32(garble_circuit *gc, garble_context *ctxt, const int inputs[3],
              int outputs[2]);
void
circuit_add22(garble_circuit *gc, garble_context *ctxt, const int inputs[2],
              int outputs[2]);

/* AES components */

void
aescircuit_add_round_key(garble_circuit *gc, garble_context *ctxt,
                         const int inputs[256], int outputs[128]);
void
aescircuit_sub_bytes(garble_circuit *gc, garble_context *ctxt,
                     const int inputs[8], int outputs[8]);
void
aescircuit_shift_rows(const int *inputs, int *outputs);
void
aescircuit_mix_columns(garble_circuit *gc, garble_context *ctxt,
                       const int *inputs, int *outputs);

#endif /* CIRCUITS_H_ */

