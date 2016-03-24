#include <garble.h>
#include "gates.h"

static void
_gate(garble_circuit *gc, garble_context *ctxt, int input0, int input1,
      int output, garble_gate_type_e type)
{
    garble_gate *g;

    if (ctxt->n_gates == gc->q) {
        ctxt->n_gates += 100;
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
