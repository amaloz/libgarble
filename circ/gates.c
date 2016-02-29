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
#include "garble/gates.h"

static void
genericGate(GarbledCircuit *gc, int input0, int input1, int output, int type)
{
    Gate *g;

    g = &gc->gates[gc->q++];
    g->type = type;
    g->input0 = input0;
    g->input1 = input1;
    g->output = output;
}

void
ANDGate(GarbledCircuit *gc, GarblingContext *ctxt,
        int input0, int input1, int output)
{
    genericGate(gc, input0, input1, output, ANDGATE);
}

void
XORGate(GarbledCircuit *gc, GarblingContext *ctxt,
        int input0, int input1, int output)
{
    genericGate(gc, input0, input1, output, XORGATE);
}

void
ORGate(GarbledCircuit *gc, GarblingContext *ctxt,
       int input0, int input1, int output)
{
    genericGate(gc, input0, input1, output, ORGATE);
}

void
NOTGate(GarbledCircuit *gc, GarblingContext *ctxt,
        int input0, int output)
{
    genericGate(gc, input0, input0, output, NOTGATE);
}

int
fixedZeroWire(GarbledCircuit *gc, GarblingContext *ctxt)
{
    int ind = garble_next_wire(ctxt);
    gc->fixedWires[gc->nFixedWires].type = FIXED_WIRE_ZERO;
    gc->fixedWires[gc->nFixedWires].idx = ind;
    gc->nFixedWires++;
    return ind;

}
int
fixedOneWire(GarbledCircuit *gc, GarblingContext *ctxt)
{
    int ind = garble_next_wire(ctxt);
    gc->fixedWires[gc->nFixedWires].type = FIXED_WIRE_ONE;
    gc->fixedWires[gc->nFixedWires].idx = ind;
    gc->nFixedWires++;
    return ind;
}
