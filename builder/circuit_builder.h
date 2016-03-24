#ifndef LIBGARBLEC_BUILDER_H
#define LIBGARBLEC_BUILDER_H

#include "garble.h"

/* Context info for building a circuit description */
typedef struct {
    uint64_t wire_index;
    uint64_t n_gates;
} garble_context;

/* Call this before constructing a new circuit */
void
builder_start_building(garble_circuit *gc, garble_context *ctxt);
/* Call this after completing the construction of a circuit */
void
builder_finish_building(garble_circuit *gc, garble_context *ctxt,
                       const int *outputs);

void
builder_init_wires(int *wires, uint64_t n);
int
builder_next_wire(garble_context *ctxt);

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
