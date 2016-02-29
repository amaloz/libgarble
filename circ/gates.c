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

#include <garble.h>
#include "gates.h"

static void
_gate(garble_circuit *gc, int input0, int input1, int output,
      garble_gate_type_e type)
{
    garble_gate *g;

    g = &gc->gates[gc->q++];
    g->type = type;
    g->input0 = input0;
    g->input1 = input1;
    g->output = output;
}

void
garble_gate_AND(garble_circuit *gc, int input0, int input1, int output)
{
    _gate(gc, input0, input1, output, GARBLE_GATE_AND);
}

void
garble_gate_XOR(garble_circuit *gc, int input0, int input1, int output)
{
    _gate(gc, input0, input1, output, GARBLE_GATE_XOR);
}

void
garble_gate_OR(garble_circuit *gc, int input0, int input1, int output)
{
    _gate(gc, input0, input1, output, GARBLE_GATE_OR);
}

void
garble_gate_NOT(garble_circuit *gc, int input0, int output)
{
    _gate(gc, input0, input0, output, GARBLE_GATE_NOT);
}

int
garble_gate_zero(garble_circuit *gc, garble_context *ctxt)
{
    int ind = garble_next_wire(ctxt);
    gc->fixedWires[gc->nFixedWires].type = GARBLE_FIXED_WIRE_ZERO;
    gc->fixedWires[gc->nFixedWires].idx = ind;
    gc->nFixedWires++;
    return ind;

}
int
garble_gate_one(garble_circuit *gc, garble_context *ctxt)
{
    int ind = garble_next_wire(ctxt);
    gc->fixedWires[gc->nFixedWires].type = GARBLE_FIXED_WIRE_ONE;
    gc->fixedWires[gc->nFixedWires].idx = ind;
    gc->nFixedWires++;
    return ind;
}
