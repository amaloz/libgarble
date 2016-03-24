#ifndef LIBGARBLEC_GATES_H
#define LIBGARBLEC_GATES_H

#include "garble.h"

void
garble_gate_AND(garble_circuit *gc, garble_context *ctxt, int input0,
                int input1, int output);
void
garble_gate_OR(garble_circuit *gc, garble_context *ctxt, int input0,
               int input1, int output);
void
garble_gate_XOR(garble_circuit *gc, garble_context *ctxt, int input0,
                int input1, int output);
void
garble_gate_NOT(garble_circuit *gc, garble_context *ctxt, int input0,
                int output);
int
garble_gate_zero(garble_circuit *gc);
int
garble_gate_one(garble_circuit *gc);

#endif
