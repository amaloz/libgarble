#ifndef LIBGARBLEC_GATES_H
#define LIBGARBLEC_GATES_H

#include "garble.h"

void
gate_AND(garble_circuit *gc, garble_context *ctxt, int input0,
         int input1, int output);
void
gate_OR(garble_circuit *gc, garble_context *ctxt, int input0,
        int input1, int output);
void
gate_XOR(garble_circuit *gc, garble_context *ctxt, int input0,
         int input1, int output);
void
gate_NOT(garble_circuit *gc, garble_context *ctxt, int input0, int output);
int
wire_zero(garble_circuit *gc);
int
wire_one(garble_circuit *gc);

#endif
