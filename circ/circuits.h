#ifndef LIBGARBLEC_CIRCUITS_H
#define LIBGARBLEC_CIRCUITS_H

#include <garble.h>

int
MULTGF16(GarbledCircuit *gc, GarblingContext *ctxt, int* inputs,
         int* outputs);

int
MUX21Circuit(GarbledCircuit *gc, GarblingContext *ctxt, int theSwitch,
             int input0, int input1, int *output);

int
SHLCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
SHRCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
MULCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);

int
XORCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n, const int *inputs,
           int *outputs);
int
NOTCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n, const int *inputs,
           int *outputs);

void
ANDCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n, const int *inputs,
           int *outputs);
void
ORCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n, const int *inputs,
          int *outputs);

int
MIXEDCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
             int *inputs, int *outputs);
int
INCCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
EQUCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
LEQCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
GEQCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
LESCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
GRECircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
MultiXORCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int d,
                int n, int *inputs, int *outputs);

int
MINCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);

int
SBOXNOTABLE(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
            int *outputs);
void
AddRoundKey(GarbledCircuit *gc, GarblingContext *ctxt, const int inputs[256],
            int outputs[128]);
void
SubBytes(GarbledCircuit *gc, GarblingContext *ctxt, const int inputs[8],
         int outputs[8]);
int
SubBytesTable(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
              int *outputs);
int
ShiftRows(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
          int *outputs);
int
MixColumns(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
           int *outputs);
int
ADDCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
SUBCircuit(GarbledCircuit *gc, GarblingContext *ctxt, int n,
           int *inputs, int *outputs);
int
ADD32Circuit(GarbledCircuit *gc, GarblingContext *ctxt,
             int *inputs, int *outputs);
int
ADD22Circuit(GarbledCircuit *gc, GarblingContext *ctxt,
             int *inputs, int *outputs);
int
MULTE_GF16(GarbledCircuit *gc, GarblingContext *ctxt,
           int *inputs, int *outputs);
int
INV_GF16(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
         int *outputs);
int
AFFINE(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
       int *outputs);
int
SBOX(GarbledCircuit *gc, GarblingContext *ctxt,
     int *inputs, int *outputs);
int
INVMAP(GarbledCircuit *gc, GarblingContext *ctxt, int *inputs,
       int *outputs);
int
GF8MULCircuit(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
              int *outputs);

void
GF4MULCircuit(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
              int *outputs);
void
GF4SCLNCircuit(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
               int *outputs);
void
GF4SQCircuit(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
             int *outputs);
int
GF4SCLN2Circuit(GarbledCircuit *gc, GarblingContext *ctxt,
                int *inputs, int *outputs);

int
RANDCircuit(GarbledCircuit *gc, GarblingContext *ctxt,
            int n, int *inputs, int *outputs, int q, int qf);

void
GF256InvCircuit(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
                int *outputs);
void
GF16INVCircuit(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
               int *outputs);


#endif /* CIRCUITS_H_ */

