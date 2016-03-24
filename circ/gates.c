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
garble_gate_AND(garble_circuit *gc, garble_context *ctxt, int input0,
                int input1, int output)
{
    _gate(gc, ctxt, input0, input1, output, GARBLE_GATE_AND);
}

void
garble_gate_XOR(garble_circuit *gc, garble_context *ctxt, int input0,
                int input1, int output)
{
    _gate(gc, ctxt, input0, input1, output, GARBLE_GATE_XOR);
}

void
garble_gate_OR(garble_circuit *gc, garble_context *ctxt, int input0,
               int input1, int output)
{
    _gate(gc, ctxt, input0, input1, output, GARBLE_GATE_OR);
}

void
garble_gate_NOT(garble_circuit *gc, garble_context *ctxt, int input0,
                int output)
{
    _gate(gc, ctxt, input0, input0, output, GARBLE_GATE_NOT);
}

int
garble_gate_zero(garble_circuit *gc)
{
    return gc->n;
}
int
garble_gate_one(garble_circuit *gc)
{
    return gc->n + 1;
}
