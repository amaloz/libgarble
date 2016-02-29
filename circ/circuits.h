#ifndef LIBGARBLEC_CIRCUITS_H
#define LIBGARBLEC_CIRCUITS_H

#include <garble.h>

int
MULTGF16(garble_circuit *gc, garble_context *ctxt, int* inputs,
         int* outputs);

int
MUX21Circuit(garble_circuit *gc, garble_context *ctxt, int theSwitch,
             int input0, int input1, int *output);

int
SHLCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
SHRCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
MULCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);

int
XORCircuit(garble_circuit *gc, garble_context *ctxt, int n, const int *inputs,
           int *outputs);
int
NOTCircuit(garble_circuit *gc, garble_context *ctxt, int n, const int *inputs,
           int *outputs);

void
ANDCircuit(garble_circuit *gc, garble_context *ctxt, int n, const int *inputs,
           int *outputs);
void
ORCircuit(garble_circuit *gc, garble_context *ctxt, int n, const int *inputs,
          int *outputs);

int
MIXEDCircuit(garble_circuit *gc, garble_context *ctxt, int n,
             int *inputs, int *outputs);
int
INCCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
EQUCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
LEQCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
GEQCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
LESCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
GRECircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
MultiXORCircuit(garble_circuit *gc, garble_context *ctxt, int d,
                int n, int *inputs, int *outputs);

int
MINCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);

int
SBOXNOTABLE(garble_circuit *gc, garble_context *ctxt, const int *inputs,
            int *outputs);
void
AddRoundKey(garble_circuit *gc, garble_context *ctxt, const int inputs[256],
            int outputs[128]);
void
SubBytes(garble_circuit *gc, garble_context *ctxt, const int inputs[8],
         int outputs[8]);
int
SubBytesTable(garble_circuit *gc, garble_context *ctxt, const int *inputs,
              int *outputs);
int
ShiftRows(garble_circuit *gc, garble_context *ctxt, const int *inputs,
          int *outputs);
int
MixColumns(garble_circuit *gc, garble_context *ctxt, const int *inputs,
           int *outputs);
int
ADDCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
SUBCircuit(garble_circuit *gc, garble_context *ctxt, int n,
           int *inputs, int *outputs);
int
ADD32Circuit(garble_circuit *gc, garble_context *ctxt,
             int *inputs, int *outputs);
int
ADD22Circuit(garble_circuit *gc, garble_context *ctxt,
             int *inputs, int *outputs);
int
MULTE_GF16(garble_circuit *gc, garble_context *ctxt,
           int *inputs, int *outputs);
int
INV_GF16(garble_circuit *gc, garble_context *ctxt, const int *inputs,
         int *outputs);
int
AFFINE(garble_circuit *gc, garble_context *ctxt, const int *inputs,
       int *outputs);
int
SBOX(garble_circuit *gc, garble_context *ctxt,
     int *inputs, int *outputs);
int
INVMAP(garble_circuit *gc, garble_context *ctxt, int *inputs,
       int *outputs);
int
GF8MULCircuit(garble_circuit *gc, garble_context *ctxt, const int *inputs,
              int *outputs);

void
GF4MULCircuit(garble_circuit *gc, garble_context *ctxt, const int *inputs,
              int *outputs);
void
GF4SCLNCircuit(garble_circuit *gc, garble_context *ctxt, const int *inputs,
               int *outputs);
void
GF4SQCircuit(garble_circuit *gc, garble_context *ctxt, const int *inputs,
             int *outputs);
int
GF4SCLN2Circuit(garble_circuit *gc, garble_context *ctxt,
                int *inputs, int *outputs);

int
RANDCircuit(garble_circuit *gc, garble_context *ctxt,
            int n, int *inputs, int *outputs, int q, int qf);

void
GF256InvCircuit(garble_circuit *gc, garble_context *ctxt, const int *inputs,
                int *outputs);
void
GF16INVCircuit(garble_circuit *gc, garble_context *ctxt, const int *inputs,
               int *outputs);


#endif /* CIRCUITS_H_ */

