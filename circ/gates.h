#ifndef LIBGARBLEC_GATES_H
#define LIBGARBLEC_GATES_H

#include "garble.h"

void
ANDGate(garble_circuit *gc, int input0, int input1, int output);
void
ORGate(garble_circuit *gc, int input0, int input1, int output);
void
XORGate(garble_circuit *gc, int input0, int input1, int output);
void
NOTGate(garble_circuit *gc, int input0, int output);
int
fixedZeroWire(garble_circuit *gc, garble_context *ctxt);
int
fixedOneWire(garble_circuit *gc, garble_context *ctxt);

#endif /* GATES_H_ */
