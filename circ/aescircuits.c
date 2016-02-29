
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
#include "circuits.h"

/*******
 * These AES circuits were modeled after the AES circuits of
 * the MPC system due to
 * Huang, Evans, Katz and Malka, available at mightbeevil.org
 */

static int A2X1[8] = { 0x98, 0xF3, 0xF2, 0x48, 0x09, 0x81, 0xA9, 0xFF };
static int S2X1[8] = { 0x8C, 0x79, 0x05, 0xEB, 0x12, 0x04, 0x51, 0x53 };

#define fbits(v, p) ((v & (1 << p)) >> p)

static void
EncoderZeroCircuit(GarbledCircuit *gc, GarblingContext *ctxt,
                   const int inputs[8], int outputs[8], const int enc[8])
{
    int wires[8];
	for (int i = 0; i < 8; i++) {
		wires[i] = fixedZeroWire(gc, ctxt);
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (fbits(enc[i], j)) {
				int wire = garble_next_wire(ctxt);
				XORGate(gc, ctxt, wires[j], inputs[i], wire);
				wires[j] = wire;
			}
		}
	}
	for (int i = 0; i < 8; i++) {
		outputs[i] = wires[i];
	}
}

static void
EncoderOneCircuit(GarbledCircuit *gc, GarblingContext *ctxt, const int inputs[8],
                  int outputs[8], const int enc[8])
{
	int wires[8];
	for (int i = 0; i < 8; i++) {
		wires[i] = fixedOneWire(gc, ctxt);
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (fbits(enc[i], j)) {
				int wire = garble_next_wire(ctxt);
				XORGate(gc, ctxt, wires[j], inputs[i], wire);
				wires[j] = wire;
			}
		}
	}
	for (int i = 0; i < 8; i++) {
		outputs[i] = wires[i];
	}
}


void
AddRoundKey(GarbledCircuit *gc, GarblingContext *ctxt, const int inputs[256],
            int outputs[128])
{
	XORCircuit(gc, ctxt, 256, inputs, outputs);
}

void
SubBytes(GarbledCircuit *gc, GarblingContext *ctxt, const int inputs[8],
         int outputs[8])
{
    int temp1[8], temp2[8];
	EncoderZeroCircuit(gc, ctxt, inputs, temp1, A2X1);
	GF256InvCircuit(gc, ctxt, temp1, temp2);
	EncoderOneCircuit(gc, ctxt, temp2, outputs, S2X1);
}

int
ShiftRows(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
          int *outputs)
{
	static int shiftTable[] =
        { 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11 };
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++)
			outputs[8 * i + j] = inputs[shiftTable[i] * 8 + j];
	}
	return 0;
}

int
MixColumns(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
           int *outputs)
{
	int mulOut[4][8];
	unsigned j;
	int inp[4][40];

	for (int i = 0; i < 4; i++)
		GF8MULCircuit(gc, ctxt, inputs + 8 * i, mulOut[i]);

	for (int i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++)
			inp[i][j] = mulOut[i][j];
		for (j = 0; j < 8; j++)
			inp[i][8 + j] = mulOut[(i + 1) % 4][j];
		for (j = 0; j < 8; j++)
			inp[i][16 + j] = inputs[((i + 1) % 4) * 8 + j];
		for (j = 0; j < 8; j++)
			inp[i][24 + j] = inputs[((i + 2) % 4) * 8 + j];
		for (j = 0; j < 8; j++)
			inp[i][32 + j] = inputs[((i + 3) % 4) * 8 + j];
	}

	for (int i = 0; i < 4; i++)
		MultiXORCircuit(gc, ctxt, 5, 40, inp[i], outputs + 8 * i);

	return 0;
}

static int
MAP(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs, int *outputs)
{
	unsigned char A = 0;
	unsigned char B = 1;
	unsigned char C = 2;
	unsigned char L0 = 3;
	unsigned char L1 = 4;
	unsigned char L3 = 5;
	unsigned char H0 = 6;
	unsigned char H1 = 7;
	unsigned char H2 = 8;

	int i;
	int tempW[10];
	for (i = 0; i < 10; i++)
		tempW[i] = garble_next_wire(ctxt);

	XORGate(gc, ctxt, inputs[1], inputs[7], tempW[A]);

	XORGate(gc, ctxt, inputs[5], inputs[7], tempW[B]);

	XORGate(gc, ctxt, inputs[4], inputs[6], tempW[C]);

	int temp = garble_next_wire(ctxt);
	XORGate(gc, ctxt, tempW[C], inputs[0], temp);
	XORGate(gc, ctxt, temp, inputs[5], tempW[L0]);

	XORGate(gc, ctxt, inputs[1], inputs[2], tempW[L1]);

	XORGate(gc, ctxt, inputs[4], inputs[2], tempW[L3]);

	XORGate(gc, ctxt, inputs[5], tempW[C], tempW[H0]);

	XORGate(gc, ctxt, inputs[A], inputs[C], tempW[H1]);

	int temp2 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, tempW[B], inputs[2], temp2);
	XORGate(gc, ctxt, temp2, inputs[3], tempW[H2]);

	outputs[0] = tempW[L0];
	outputs[1] = tempW[L1];
	outputs[2] = tempW[A];
	outputs[3] = tempW[L3];
	outputs[4] = tempW[H0];
	outputs[5] = tempW[H1];
	outputs[6] = tempW[H2];
	outputs[7] = tempW[B];
	return 0;
}

int
INVMAP(GarbledCircuit *gc, GarblingContext *ctxt, int* inputs,
       int* outputs)
{
	unsigned char A = 0;
	unsigned char B = 1;
	unsigned char a0 = 2;
	unsigned char a1 = 3;
	unsigned char a2 = 4;
	unsigned char a3 = 5;
	unsigned char a4 = 6;
	unsigned char a5 = 7;
	unsigned char a6 = 8;
	unsigned char a7 = 9;

	unsigned char l0 = 0;
	unsigned char l1 = 1;
	unsigned char l2 = 2;
	unsigned char l3 = 3;
	unsigned char h0 = 4;
	unsigned char h1 = 5;
	unsigned char h2 = 6;
	unsigned char h3 = 7;

	int i;
	for (i = 0; i < 8; i++)
		outputs[i] = garble_next_wire(ctxt);
	int tempW[16];
	for (i = 0; i < 16; i++)
		tempW[i] = garble_next_wire(ctxt);

	XORGate(gc, ctxt, inputs[l1], inputs[h3], tempW[A]);

	XORGate(gc, ctxt, inputs[h1], inputs[h0], tempW[B]);

	XORGate(gc, ctxt, inputs[l0], inputs[h0], tempW[a0]);

	XORGate(gc, ctxt, inputs[h3], tempW[B], tempW[a1]);

	XORGate(gc, ctxt, tempW[A], tempW[B], tempW[a2]);

	int temp = garble_next_wire(ctxt);
	XORGate(gc, ctxt, tempW[B], inputs[l1], temp);

	XORGate(gc, ctxt, temp, inputs[h2], tempW[a3]);

	temp = garble_next_wire(ctxt);
	XORGate(gc, ctxt, tempW[A], tempW[B], temp);

	XORGate(gc, ctxt, inputs[l3], temp, tempW[a4]);

	XORGate(gc, ctxt, inputs[l2], tempW[B], tempW[a5]);

	temp = garble_next_wire(ctxt);
	XORGate(gc, ctxt, tempW[A], inputs[l2], temp);

	int temp2 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, temp, inputs[l3], temp2);
	XORGate(gc, ctxt, temp2, inputs[h0], tempW[a6]);

	temp2 = garble_next_wire(ctxt);

	XORGate(gc, ctxt, tempW[B], inputs[l2], temp2);

	XORGate(gc, ctxt, temp2, inputs[h3], tempW[a7]);

	outputs[0] = tempW[a0];
	outputs[1] = tempW[a1];
	outputs[2] = tempW[a2];
	outputs[3] = tempW[a3];
	outputs[4] = tempW[a4];
	outputs[5] = tempW[a5];
	outputs[6] = tempW[a6];
	outputs[7] = tempW[a7];
	return 0;

}

int
MULTGF16(GarbledCircuit *gc, GarblingContext *ctxt, int* inputs,
         int* outputs)
{
	unsigned char A = 0;
	unsigned char B = 1;
	unsigned char C = 2;
	unsigned char q0 = 3;
	unsigned char q1 = 4;
	unsigned char q2 = 5;
	unsigned char q3 = 6;

	unsigned char and00 = 7;
	unsigned char and31 = 8;
	unsigned char and22 = 9;
	unsigned char and13 = 10;
	unsigned char and10 = 11;
	unsigned char andA1 = 12;
	unsigned char andB2 = 13;
	unsigned char andC3 = 14;
	unsigned char and20 = 15;
	unsigned char and11 = 16;
	unsigned char andA2 = 17;
	unsigned char andB3 = 18;
	unsigned char and30 = 19;
	unsigned char and21 = 20;
	unsigned char and12 = 21;
	unsigned char andA3 = 22;

	unsigned char b0 = 0;
	unsigned char b1 = 1;
	unsigned char b2 = 2;
	unsigned char b3 = 3;
	unsigned char a0 = 4;
	unsigned char a1 = 5;
	unsigned char a2 = 6;
	unsigned char a3 = 7;

	int tempW[24];
	int i;
	for (i = 0; i < 24; i++)
		tempW[i] = garble_next_wire(ctxt);

	XORGate(gc, ctxt, inputs[a3], inputs[a0], tempW[A]);

	XORGate(gc, ctxt, inputs[a3], inputs[a2], tempW[B]);

	XORGate(gc, ctxt, inputs[a1], inputs[a2], tempW[C]);

	ANDGate(gc, ctxt, inputs[a0], inputs[b0], tempW[and00]);

	ANDGate(gc, ctxt, inputs[a3], inputs[b1], tempW[and31]);

	ANDGate(gc, ctxt, inputs[a2], inputs[b2], tempW[and22]);

	ANDGate(gc, ctxt, inputs[a1], inputs[b3], tempW[and13]);

	int temp1 = garble_next_wire(ctxt);
	int temp2 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, tempW[and00], tempW[and31], temp1);

	XORGate(gc, ctxt, tempW[and13], tempW[and22], temp2);
	XORGate(gc, ctxt, temp1, temp2, tempW[q0]);

	ANDGate(gc, ctxt, inputs[a1], inputs[b0], tempW[and10]);

	ANDGate(gc, ctxt, tempW[A], inputs[b1], tempW[andA1]);

	ANDGate(gc, ctxt, tempW[B], inputs[b2], tempW[andB2]);

	ANDGate(gc, ctxt, tempW[C], inputs[b3], tempW[andC3]);

	temp1 = garble_next_wire(ctxt);
	temp2 = garble_next_wire(ctxt);

	XORGate(gc, ctxt, tempW[and10], tempW[andA1], temp1);

	XORGate(gc, ctxt, tempW[andB2], tempW[andC3], temp2);

	XORGate(gc, ctxt, temp1, temp2, tempW[q1]);

	ANDGate(gc, ctxt, inputs[a2], inputs[b0], tempW[and20]);

	ANDGate(gc, ctxt, inputs[a1], inputs[b1], tempW[and11]);

	ANDGate(gc, ctxt, tempW[A], inputs[b2], tempW[andA2]);

	ANDGate(gc, ctxt, tempW[B], inputs[b3], tempW[andB3]);

	temp1 = garble_next_wire(ctxt);
	temp2 = garble_next_wire(ctxt);

	XORGate(gc, ctxt, tempW[and20], tempW[and11], temp1);

	XORGate(gc, ctxt, tempW[andA2], tempW[andB3], temp2);

	XORGate(gc, ctxt, temp1, temp2, tempW[q2]);

	ANDGate(gc, ctxt, inputs[a3], inputs[b0], tempW[and30]);

	ANDGate(gc, ctxt, inputs[a2], inputs[b1], tempW[and21]);

	ANDGate(gc, ctxt, inputs[a1], inputs[b2], tempW[and12]);

	ANDGate(gc, ctxt, tempW[A], inputs[b3], tempW[andA3]);

	temp1 = garble_next_wire(ctxt);
	temp2 = garble_next_wire(ctxt);

	XORGate(gc, ctxt, tempW[and30], tempW[and21], temp1);

	XORGate(gc, ctxt, tempW[andA3], tempW[and12], temp2);

	XORGate(gc, ctxt, temp1, temp2, tempW[q3]);

	outputs[0] = tempW[q0];
	outputs[1] = tempW[q1];
	outputs[2] = tempW[q2];
	outputs[3] = tempW[q3];
	return 0;

}

int
SquareCircuit(GarbledCircuit *gc,
              GarblingContext *ctxt, int n, int* inputs, int* outputs)
{
	outputs[0] = garble_next_wire(ctxt);
	XORGate(gc, ctxt, inputs[0], inputs[2], outputs[0]);

	outputs[1] = inputs[2];
	outputs[2] = garble_next_wire(ctxt);
	XORGate(gc, ctxt, inputs[1], inputs[3], outputs[2]);

	outputs[3] = inputs[3];
	return 0;
}

int
MULTE_GF16(GarbledCircuit *gc, GarblingContext *ctxt,
           int* inputs, int* outputs)
{
	int outputA = garble_next_wire(ctxt);
	int outputB = garble_next_wire(ctxt);
	int outputq0 = garble_next_wire(ctxt);
	int outputq2 = garble_next_wire(ctxt);
	int outputq3 = garble_next_wire(ctxt);

	XORGate(gc, ctxt, inputs[0], inputs[1], outputA);
	XORGate(gc, ctxt, inputs[2], inputs[3], outputB);
	XORGate(gc, ctxt, outputB, inputs[1], outputq0);
	XORGate(gc, ctxt, outputA, inputs[2], outputq2);
	XORGate(gc, ctxt, outputB, outputA, outputq3);

	outputs[0] = outputq0;
	outputs[1] = outputA;
	outputs[2] = outputq2;
	outputs[3] = outputq3;
	return 0;

}

int
INV_GF16(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
         int *outputs)
{
	int AOutput = garble_next_wire(ctxt);
	int q0Output = garble_next_wire(ctxt);
	int q1Output = garble_next_wire(ctxt);
	int q2Output = garble_next_wire(ctxt);
	int q3Output = garble_next_wire(ctxt);

	int and01Output = garble_next_wire(ctxt);
	int and02Output = garble_next_wire(ctxt);
	int and03Output = garble_next_wire(ctxt);
	int and12Output = garble_next_wire(ctxt);
	int and13Output = garble_next_wire(ctxt);
	int and23Output = garble_next_wire(ctxt);
	int and012Output = garble_next_wire(ctxt);
	int and123Output = garble_next_wire(ctxt);
	int and023Output = garble_next_wire(ctxt);
	int and013Output = garble_next_wire(ctxt);

	ANDGate(gc, ctxt, inputs[0], inputs[1], and01Output);
	ANDGate(gc, ctxt, inputs[0], inputs[2], and02Output);
	ANDGate(gc, ctxt, inputs[0], inputs[3], and03Output);
	ANDGate(gc, ctxt, inputs[2], inputs[1], and12Output);
	ANDGate(gc, ctxt, inputs[3], inputs[1], and13Output);
	ANDGate(gc, ctxt, inputs[2], inputs[3], and23Output);
	ANDGate(gc, ctxt, inputs[2], and01Output, and012Output);
	ANDGate(gc, ctxt, inputs[3], and12Output, and123Output);
	ANDGate(gc, ctxt, inputs[3], and02Output, and023Output);
	ANDGate(gc, ctxt, inputs[3], and01Output, and013Output);

	int tempXORA1 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, inputs[1], inputs[2], tempXORA1);
	int tempXORA2 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, inputs[3], and123Output, tempXORA2);
	XORGate(gc, ctxt, tempXORA1, tempXORA2, AOutput);

	int tempXORq01 = garble_next_wire(ctxt);
	int tempXORq02 = garble_next_wire(ctxt);
	int tempXORq03 = garble_next_wire(ctxt);

	XORGate(gc, ctxt, inputs[0], AOutput, tempXORq01);
	XORGate(gc, ctxt, and02Output, tempXORq01, tempXORq02);
	XORGate(gc, ctxt, and12Output, tempXORq02, tempXORq03);
	XORGate(gc, ctxt, and012Output, tempXORq03, q0Output);

	int tempXORq11 = garble_next_wire(ctxt);
	int tempXORq12 = garble_next_wire(ctxt);
	int tempXORq13 = garble_next_wire(ctxt);
	int tempXORq14 = garble_next_wire(ctxt);
	/* int tempXORq15 = */ garble_next_wire(ctxt);

	XORGate(gc, ctxt, and01Output, and02Output, tempXORq11);
	XORGate(gc, ctxt, and12Output, tempXORq11, tempXORq12);
	XORGate(gc, ctxt, inputs[3], tempXORq12, tempXORq13);
	XORGate(gc, ctxt, and13Output, tempXORq13, tempXORq14);
	XORGate(gc, ctxt, and013Output, tempXORq14, q1Output);

	int tempXORq21 = garble_next_wire(ctxt);
	int tempXORq22 = garble_next_wire(ctxt);
	int tempXORq23 = garble_next_wire(ctxt);
	int tempXORq24 = garble_next_wire(ctxt);
	/* int tempXORq25 = */ garble_next_wire(ctxt);

	XORGate(gc, ctxt, and01Output, inputs[2], tempXORq21);
	XORGate(gc, ctxt, and02Output, tempXORq21, tempXORq22);
	XORGate(gc, ctxt, inputs[3], tempXORq22, tempXORq23);
	XORGate(gc, ctxt, and03Output, tempXORq23, tempXORq24);
	XORGate(gc, ctxt, and023Output, tempXORq24, q2Output);

	int tempXORq31 = garble_next_wire(ctxt);
	int tempXORq32 = garble_next_wire(ctxt);
	/* int tempXORq33 = */ garble_next_wire(ctxt);
	/* int tempXORq34 = */ garble_next_wire(ctxt);
	/* int tempXORq35 = */ garble_next_wire(ctxt);

	XORGate(gc, ctxt, AOutput, and03Output, tempXORq31);
	XORGate(gc, ctxt, and13Output, tempXORq31, tempXORq32);
	XORGate(gc, ctxt, and23Output, tempXORq32, q3Output);

	outputs[0] = q0Output;
	outputs[1] = q1Output;
	outputs[2] = q2Output;
	outputs[3] = q3Output;

	return 0;
}

int
AFFINE(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
       int *outputs)
{
	int AOutput = garble_next_wire(ctxt);
	int BOutput = garble_next_wire(ctxt);
	int COutput = garble_next_wire(ctxt);
	int DOutput = garble_next_wire(ctxt);

	int q0Output = garble_next_wire(ctxt);
	int q1Output = garble_next_wire(ctxt);
	int q2Output = garble_next_wire(ctxt);
	int q3Output = garble_next_wire(ctxt);
	int q4Output = garble_next_wire(ctxt);
	int q5Output = garble_next_wire(ctxt);
	int q6Output = garble_next_wire(ctxt);
	int q7Output = garble_next_wire(ctxt);

	int a0barOutput = garble_next_wire(ctxt);
	int a1barOutput = garble_next_wire(ctxt);
	int a5barOutput = garble_next_wire(ctxt);
	int a6barOutput = garble_next_wire(ctxt);

	XORGate(gc, ctxt, inputs[0], inputs[1], AOutput);
	XORGate(gc, ctxt, inputs[2], inputs[3], BOutput);
	XORGate(gc, ctxt, inputs[4], inputs[5], COutput);
	XORGate(gc, ctxt, inputs[6], inputs[7], DOutput);
	NOTGate(gc, ctxt, inputs[0], a0barOutput);
	NOTGate(gc, ctxt, inputs[1], a1barOutput);
	NOTGate(gc, ctxt, inputs[5], a5barOutput);
	NOTGate(gc, ctxt, inputs[6], a6barOutput);

	int tempWireq0 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, a0barOutput, COutput, tempWireq0);
	XORGate(gc, ctxt, tempWireq0, DOutput, q0Output);

	int tempWireq1 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, a5barOutput, AOutput, tempWireq1);
	XORGate(gc, ctxt, tempWireq1, DOutput, q1Output);

	int tempWireq2 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, inputs[2], AOutput, tempWireq2);
	XORGate(gc, ctxt, tempWireq2, DOutput, q2Output);

	int tempWireq3 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, inputs[7], AOutput, tempWireq3);
	XORGate(gc, ctxt, tempWireq3, BOutput, q3Output);

	int tempWireq4 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, inputs[4], AOutput, tempWireq4);
	XORGate(gc, ctxt, tempWireq4, BOutput, q4Output);

	int tempWireq5 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, a1barOutput, BOutput, tempWireq5);
	XORGate(gc, ctxt, tempWireq5, COutput, q5Output);

	int tempWireq6 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, a6barOutput, BOutput, tempWireq6);
	XORGate(gc, ctxt, tempWireq6, COutput, q6Output);

	int tempWireq7 = garble_next_wire(ctxt);
	XORGate(gc, ctxt, inputs[3], COutput, tempWireq7);
	XORGate(gc, ctxt, tempWireq7, DOutput, q7Output);

	outputs[0] = q0Output;
	outputs[1] = q1Output;
	outputs[2] = q2Output;
	outputs[3] = q3Output;
	outputs[4] = q4Output;
	outputs[5] = q5Output;
	outputs[6] = q6Output;
	outputs[7] = q7Output;

	return 0;
}

static int
INV_GF256(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
          int *outputs)
{
	int i;
	int square0Inputs[8];
	int square0Outputs[8];
	int square1Inputs[8];
	int square1Outputs[8];

	int Mult0Inputs[8];
	int Mult1Inputs[8];
	int XOR0Inputs[8];
	int XOR0Outputs[8];
	int XOR1Inputs[8];
	int XOR1Outputs[8];
	int Mult0Outputs[8];
	int Mult1Outputs[8];

	int mapOutputs[16];

	MAP(gc, ctxt, inputs, mapOutputs);

	for (i = 0; i < 4; i++) {
		square0Inputs[i] = mapOutputs[i + 4];

		square1Inputs[i] = mapOutputs[i];

		Mult0Inputs[i] = mapOutputs[i + 4];

		Mult0Inputs[i + 4] = mapOutputs[i];

		XOR0Inputs[i] = mapOutputs[i + 4];

		XOR0Inputs[i + 4] = mapOutputs[i];
	}

	SquareCircuit(gc, ctxt, 4, square0Inputs,
			square0Outputs);
	SquareCircuit(gc, ctxt, 4, square1Inputs,
			square1Outputs);

	MULTGF16(gc, ctxt, Mult0Inputs, Mult0Outputs);

	XORCircuit(gc, ctxt, 8, XOR0Inputs, XOR0Outputs);

	int MultEInputs[8];
	int MultEOutputs[8];
	int XOR2Inputs[8];
	int XOR2Outputs[8];

	for (i = 0; i < 4; i++) {

		MultEInputs[i] = square0Outputs[i];
		XOR1Inputs[i] = square1Outputs[i];
	}

	MULTE_GF16(gc, ctxt, MultEInputs, MultEOutputs);

	for (i = 0; i < 4; i++) {
		XOR1Inputs[i + 4] = MultEOutputs[i];
	}

	XORCircuit(gc, ctxt, 8, XOR1Inputs, XOR1Outputs);
	for (i = 0; i < 4; i++) {
		XOR2Inputs[i] = Mult0Outputs[i];
		XOR2Inputs[i + 4] = XOR1Outputs[i];
	}
	XORCircuit(gc, ctxt, 8, XOR2Inputs, XOR2Outputs);

	int InvtInputs[8];
	int InvtOutputs[8];

	for (i = 0; i < 4; i++) {
		InvtInputs[i] = XOR2Outputs[i];
	}
	INV_GF16(gc, ctxt, InvtInputs, InvtOutputs);

	for (i = 0; i < 4; i++) {
		Mult1Inputs[i] = InvtOutputs[i];
	}
	for (i = 0; i < 4; i++) {
		Mult1Inputs[i + 4] = mapOutputs[i + 4];
	}
	MULTGF16(gc, ctxt, Mult1Inputs, Mult1Outputs);

	int Mult2Inputs[8];
	int Mult2Outputs[8];
	for (i = 0; i < 4; i++) {
		Mult2Inputs[i] = XOR0Outputs[i];
		Mult2Inputs[i + 4] = InvtOutputs[i];
	}
	MULTGF16(gc, ctxt, Mult2Inputs, Mult2Outputs);

	int InvMapInputs[8];
	int InvMapOutputs[8];

	for (i = 0; i < 4; i++) {
		InvMapInputs[i] = Mult2Outputs[i];
		InvMapInputs[i + 4] = Mult1Outputs[i];
	}

	INVMAP(gc, ctxt, InvMapInputs, InvMapOutputs);
	for (i = 0; i < 8; i++) {
		outputs[i] = InvMapOutputs[i];
	}
	return 0;
}

int
SBOXNOTABLE(GarbledCircuit *gc, GarblingContext *ctxt, const int *inputs,
            int *outputs)
{
	int invInputs[8];
	int invOutputs[8];
	for (int i = 0; i < 8; i++)
		invInputs[i] = inputs[i];
	INV_GF256(gc, ctxt, invInputs, invOutputs);
	AFFINE(gc, ctxt, invOutputs, outputs);
	return 0;
}
