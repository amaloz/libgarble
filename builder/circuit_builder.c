#include <garble.h>
#include "circuit_builder.h"

void
builder_init_wires(int *wires, uint64_t n)
{
    for (uint64_t i = 0; i < n; i++)
        wires[i] = i;
}

int
builder_next_wire(garble_context *ctxt)
{
    return ctxt->wire_index++;
}

void
builder_start_building(garble_circuit *gc, garble_context *ctxt)
{
    ctxt->wire_index = gc->n + 2; /* start at first non-input, non-fixed wire */
    ctxt->n_gates = 0;
}

void
builder_finish_building(garble_circuit *gc, garble_context *ctxt,
                       const int *outputs)
{
    gc->r = ctxt->wire_index;
    for (uint64_t i = 0; i < gc->m; ++i) {
        gc->outputs[i] = outputs[i];
    }
}

static inline void
_gate(garble_circuit *gc, garble_context *ctxt, int input0, int input1,
      int output, garble_gate_type_e type)
{
    garble_gate *g;

    if (ctxt->n_gates == gc->q) {
        ctxt->n_gates += 1000;
        gc->gates = realloc(gc->gates, ctxt->n_gates * sizeof(garble_gate));
    }
    g = &gc->gates[gc->q++];
    g->type = type;
    g->input0 = input0;
    g->input1 = input1;
    g->output = output;
}

void
gate_AND(garble_circuit *gc, garble_context *ctxt, int input0, int input1,
         int output)
{
    _gate(gc, ctxt, input0, input1, output, GARBLE_GATE_AND);
}

void
gate_XOR(garble_circuit *gc, garble_context *ctxt, int input0, int input1,
         int output)
{
    gc->nxors++;
    _gate(gc, ctxt, input0, input1, output, GARBLE_GATE_XOR);
}

void
gate_OR(garble_circuit *gc, garble_context *ctxt, int input0, int input1,
        int output)
{
    _gate(gc, ctxt, input0, input1, output, GARBLE_GATE_OR);
}

void
gate_NOT(garble_circuit *gc, garble_context *ctxt, int input0, int output)
{
    _gate(gc, ctxt, input0, input0, output, GARBLE_GATE_NOT);
}

int
wire_zero(garble_circuit *gc)
{
    return gc->n;
}
int
wire_one(garble_circuit *gc)
{
    return gc->n + 1;
}
