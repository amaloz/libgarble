#ifndef LIBGARBLEC_CIRCUITS_H
#define LIBGARBLEC_CIRCUITS_H

#include <garble.h>

void
ANDCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs);
void
ORCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
          const int *inputs, int *outputs);
void
XORCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs);
void
NOTCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs);
void
MIXEDCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
             const int *inputs, int outputs[1]);
void
MultiXORCircuit(garble_circuit *gc, garble_context *ctxt, int d,
                uint64_t n, const int *inputs, int *outputs);

void
MUX21Circuit(garble_circuit *gc, garble_context *ctxt, 
             int theSwitch, int input0, int input1, int output[1]);

/* GF operations */

void
GF256InvCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[8],
                int outputs[8]);
void
GF16INVCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
               int outputs[4]);
void
GF16MULCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
               int outputs[4]);
void
GF16SQCLCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
                int outputs[4]);
void
GF8MULCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[8],
              int outputs[8]);
void
GF4MULCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
              int outputs[2]);
void
GF4SCLNCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[2],
               int outputs[2]);
void
GF4SCLN2Circuit(garble_circuit *gc, garble_context *ctxt,
                const int inputs[2], int outputs[2]);
void
GF4SQCircuit(const int inputs[2], int outputs[2]);

void
INCCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs);

int
SHLCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           int *inputs, int *outputs);
int
SHRCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           int *inputs, int *outputs);
int
MULCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           int *inputs, int *outputs);

int
EQUCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           int *inputs, int *outputs);
int
LEQCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           int *inputs, int *outputs);
int
GEQCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           int *inputs, int *outputs);
void
LESCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs);
void
GRECircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs);

int
MINCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
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
ShiftRows(const int *inputs, int *outputs);

int
SquareCircuit(garble_circuit *gc, garble_context *ctxt, int inputs[4],
              int outputs[4]);

int
MixColumns(garble_circuit *gc, garble_context *ctxt, const int *inputs,
           int *outputs);
int
ADDCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           int *inputs, int *outputs);
int
SUBCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           int *inputs, int *outputs);
int
ADD32Circuit(garble_circuit *gc, garble_context *ctxt,
             int *inputs, int *outputs);
int
ADD22Circuit(garble_circuit *gc, garble_context *ctxt,
             int *inputs, int *outputs);
int
MULTGF16(garble_circuit *gc, garble_context *ctxt, int* inputs,
         int* outputs);
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

void
RANDCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n, int outputs[1],
            int q, int qf);


#endif /* CIRCUITS_H_ */

