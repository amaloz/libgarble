#include <garble.h>
#include "gates.h"
#include "circuits.h"

#include <assert.h>
#include <string.h>

void
ANDCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs)
{
    assert(n >= 2);

    outputs[0] = garble_next_wire(ctxt);
    garble_gate_AND(gc, ctxt, inputs[0], inputs[1], outputs[0]);
    if (n > 2) {
        for (uint64_t i = 2; i < n; ++i) {
            int wire = garble_next_wire(ctxt);
            garble_gate_AND(gc, ctxt, inputs[i], outputs[0], wire);
            outputs[0] = wire;
        }
    }
}

void
ORCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
          const int *inputs, int *outputs)
{
    assert(n >= 2);

    outputs[0] = garble_next_wire(ctxt);
    garble_gate_OR(gc, ctxt, inputs[0], inputs[1], outputs[0]);
    if (n > 2) {
        for (uint64_t i = 2; i < n; ++i) {
            int wire = garble_next_wire(ctxt);
            garble_gate_OR(gc, ctxt, inputs[i], outputs[0], wire);
            outputs[0] = wire;
        }
    }
}

void
XORCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs)
{
    assert(n >= 2 && n % 2 == 0);
    for (uint64_t i = 0; i < n / 2; ++i) {
        int wire = garble_next_wire(ctxt);
        garble_gate_XOR(gc, ctxt, inputs[i], inputs[n / 2 + i], wire);
        outputs[i] = wire;
    }
}

void
NOTCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs)
{
    for (uint64_t i = 0; i < n; ++i) {
        outputs[i] = garble_next_wire(ctxt);
        garble_gate_NOT(gc, ctxt, inputs[i], outputs[i]);
    }
}

void
MIXEDCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
             const int *inputs, int outputs[1])
{
    int oldInternalWire = inputs[0];
    uint64_t newInternalWire;

    for (uint64_t i = 0; i < n - 1; i++) {
        newInternalWire = garble_next_wire(ctxt);
        if (i % 3 == 2)
            garble_gate_OR(gc, ctxt, inputs[i + 1], oldInternalWire, newInternalWire);
        if (i % 3 == 1)
            garble_gate_AND(gc, ctxt, inputs[i + 1], oldInternalWire, newInternalWire);
        if (i % 3 == 0)
            garble_gate_XOR(gc, ctxt, inputs[i + 1], oldInternalWire, newInternalWire);
        oldInternalWire = newInternalWire;
    }
    outputs[0] = oldInternalWire;
}

void
MultiXORCircuit(garble_circuit *gc, garble_context *ctxt, int d,
                uint64_t n, const int *inputs, int *outputs)
{
    int div = n / d;
    int *tempInWires, *tempOutWires;

    tempInWires = calloc(2 * div, sizeof(int));
    tempOutWires = calloc(div, sizeof(int));

    for (int i = 0; i < div; i++) {
        tempOutWires[i] = inputs[i];
    }

    for (int i = 1; i < d; i++) {
        for (int j = 0; j < div; j++) {
            tempInWires[j] = tempOutWires[j];
            tempInWires[div + j] = inputs[div * i + j];
        }
        XORCircuit(gc, ctxt, 2 * div, tempInWires, tempOutWires);
    }
    for (int i = 0; i < div; i++) {
        outputs[i] = tempOutWires[i];
    }

    free(tempInWires);
    free(tempOutWires);
}

void
MUX21Circuit(garble_circuit *gc, garble_context *ctxt, 
             int theSwitch, int input0, int input1, int output[1])
{
    uint64_t notSwitch = garble_next_wire(ctxt);
    garble_gate_NOT(gc, ctxt, theSwitch, notSwitch);
    int and0 = garble_next_wire(ctxt);
    garble_gate_AND(gc, ctxt, notSwitch, input0, and0);
    int and1 = garble_next_wire(ctxt);
    garble_gate_AND(gc, ctxt, theSwitch, input1, and1);
    *output = garble_next_wire(ctxt);
    garble_gate_OR(gc, ctxt, and0, and1, *output);
}

void
GF256InvCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[8],
                int outputs[8])
{
    int E[4], P[4], Q[4], tempX[4];
    int CD[8], EA[8], EB[8];

    XORCircuit(gc, ctxt, 8, inputs, tempX);
    GF16SQCLCircuit(gc, ctxt, tempX, CD);
    GF16MULCircuit(gc, ctxt, inputs, CD + 4);

    XORCircuit(gc, ctxt, 8, CD, tempX);
    GF16INVCircuit(gc, ctxt, tempX, E);

    for (int i = 0; i < 4; i++) {
        EB[i] = E[i];
        EB[i + 4] = inputs[i + 4];
    }

    for (int i = 0; i < 4; i++) {
        EA[i] = E[i];
        EA[i + 4] = inputs[i];
    }

    GF16MULCircuit(gc, ctxt, EB, P);
    GF16MULCircuit(gc, ctxt, EA, Q);

    outputs[0] = Q[0];
    outputs[1] = Q[1];
    outputs[2] = Q[2];
    outputs[3] = Q[3];
    outputs[4] = P[0];
    outputs[5] = P[1];
    outputs[6] = P[2];
    outputs[7] = P[3];
}

void
GF16INVCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
               int outputs[4])
{
    int a[2], b[2];
    int ab[4], cd[4];
    int tempx[2], tempx2[2], tempxs[2];
    int c[2], d[2], e[2], p[2], q[2];
    int eb[4], ea[4];

    a[0] = inputs[2];
    a[1] = inputs[3];
    b[0] = inputs[0];
    b[1] = inputs[1];

    ab[0] = a[0];
    ab[1] = a[1];
    ab[2] = b[0];
    ab[3] = b[1];

    XORCircuit(gc, ctxt, 4, ab, tempx);
    GF4SQCircuit(tempx, tempxs);
    GF4SCLNCircuit(gc, ctxt, tempxs, c);
    GF4MULCircuit(gc, ctxt, ab, d);

    cd[0] = c[0];
    cd[1] = c[1];
    cd[2] = d[0];
    cd[3] = d[1];

    XORCircuit(gc, ctxt, 4, cd, tempx2);
    GF4SQCircuit(tempx2, e);

    ea[0] = e[0];
    ea[1] = e[1];
    ea[2] = a[0];
    ea[3] = a[1];

    eb[0] = e[0];
    eb[1] = e[1];
    eb[2] = b[0];
    eb[3] = b[1];

    GF4MULCircuit(gc, ctxt, eb, p);
    GF4MULCircuit(gc, ctxt, ea, q);

    outputs[0] = q[0];
    outputs[1] = q[1];
    outputs[2] = p[0];
    outputs[3] = p[1];
}

void
GF16MULCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
               int outputs[4])
{
    int ab[4], cd[4], e[2];
    int abcdx[4];
    int em[2];
    int p[2], q[2];
    int ac[4];
    int bd[4];
    int tmpx1[4], tmpx2[4];

    ab[0] = inputs[2];
    ab[1] = inputs[3];
    ab[2] = inputs[0];
    ab[3] = inputs[1];

    cd[0] = inputs[2 + 4];
    cd[1] = inputs[3 + 4];
    cd[2] = inputs[0 + 4];
    cd[3] = inputs[1 + 4];

    XORCircuit(gc, ctxt, 4, ab, abcdx);
    XORCircuit(gc, ctxt, 4, cd, abcdx + 2);

    GF4MULCircuit(gc, ctxt, abcdx, e);
    GF4SCLNCircuit(gc, ctxt, e, em);

    ac[0] = ab[0];
    ac[1] = ab[1];
    ac[2] = cd[0];
    ac[3] = cd[1];

    bd[0] = ab[2 + 0];
    bd[1] = ab[2 + 1];
    bd[2] = cd[2 + 0];
    bd[3] = cd[2 + 1];

    GF4MULCircuit(gc, ctxt, ac, tmpx1);
    GF4MULCircuit(gc, ctxt, bd, tmpx2);

    tmpx1[2] = em[0];
    tmpx1[3] = em[1];
    tmpx2[2] = em[0];
    tmpx2[3] = em[1];

    XORCircuit(gc, ctxt, 4, tmpx1, p);
    XORCircuit(gc, ctxt, 4, tmpx2, q);

    outputs[0] = q[0];
    outputs[1] = q[1];
    outputs[2] = p[0];
    outputs[3] = p[1];
}

void
GF16SQCLCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
                int outputs[4])
{
    int a[2], b[2];
    int ab[4];
    int tempx[2], tempx2[4];
    int p[2], q[2];

    a[0] = inputs[2];
    a[1] = inputs[3];
    b[0] = inputs[0];
    b[1] = inputs[1];

    ab[0] = a[0];
    ab[1] = a[1];
    ab[2] = b[0];
    ab[3] = b[1];

    XORCircuit(gc, ctxt, 4, ab, tempx);

    GF4SQCircuit(tempx, p);
    GF4SQCircuit(b, tempx2);
    GF4SCLN2Circuit(gc, ctxt, tempx2, q);

    outputs[0] = q[0];
    outputs[1] = q[1];
    outputs[2] = p[0];
    outputs[3] = p[1];
}

//http://edipermadi.files.wordpress.com/2008/02/aes_galois_field.jpg
void
GF8MULCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[8],
              int outputs[8])
{
    outputs[0] = inputs[7];
    outputs[2] = inputs[1];
    outputs[3] = garble_next_wire(ctxt);
    garble_gate_XOR(gc, ctxt, inputs[7], inputs[2], outputs[3]);

    outputs[4] = garble_next_wire(ctxt);
    garble_gate_XOR(gc, ctxt, inputs[7], inputs[3], outputs[4]);

    outputs[5] = inputs[4];
    outputs[6] = inputs[5];
    outputs[7] = inputs[6];
    outputs[1] = garble_next_wire(ctxt);
    garble_gate_XOR(gc, ctxt, inputs[7], inputs[0], outputs[1]);
}

void
GF4MULCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
              int outputs[2])
{
    int a, b, c, d, e, p, q, temp1, temp2;

    a = inputs[1];
    b = inputs[0];
    c = inputs[3];
    d = inputs[2];

    temp1 = garble_next_wire(ctxt);
    garble_gate_XOR(gc, ctxt, a, b, temp1);
    temp2 = garble_next_wire(ctxt);
    garble_gate_XOR(gc, ctxt, c, d, temp2);
    e = garble_next_wire(ctxt);
    garble_gate_AND(gc, ctxt, temp1, temp2, e);
    temp1 = garble_next_wire(ctxt);
    garble_gate_AND(gc, ctxt, a, c, temp1);
    p = garble_next_wire(ctxt);
    garble_gate_XOR(gc, ctxt, temp1, e, p);
    temp2 = garble_next_wire(ctxt);
    garble_gate_AND(gc, ctxt, b, d, temp2);
    q = garble_next_wire(ctxt);
    garble_gate_XOR(gc, ctxt, temp2, e, q);

    outputs[1] = p;
    outputs[0] = q;
}

void
GF4SCLNCircuit(garble_circuit *gc, garble_context *ctxt, const int inputs[2],
               int outputs[2])
{
    outputs[0] = garble_next_wire(ctxt);
    garble_gate_XOR(gc, ctxt, inputs[0], inputs[1], outputs[0]);
    outputs[1] = inputs[0];
}

void
GF4SCLN2Circuit(garble_circuit *gc, garble_context *ctxt,
                const int inputs[2], int outputs[2])
{

    outputs[1] = garble_next_wire(ctxt);
    garble_gate_XOR(gc, ctxt, inputs[0], inputs[1], outputs[1]);
    outputs[0] = inputs[1];
}

void
GF4SQCircuit(const int inputs[2], int outputs[2])
{
    outputs[0] = inputs[1];
    outputs[1] = inputs[0];
}

void
RANDCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n, int outputs[1],
            int q, int qf)
{
    int i;
    int oldInternalWire = garble_next_wire(ctxt);
    uint64_t newInternalWire;

    garble_gate_AND(gc, ctxt, 0, 1, oldInternalWire);

    for (i = 2; i < q + qf - 1; i++) {
        newInternalWire = garble_next_wire(ctxt);
        if (i < q)
            garble_gate_AND(gc, ctxt, i % n, oldInternalWire, newInternalWire);
        else
            garble_gate_XOR(gc, ctxt, i % n, oldInternalWire, newInternalWire);
        oldInternalWire = newInternalWire;
    }
    outputs[0] = oldInternalWire;
}

void
INCCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs)
{
    for (uint64_t i = 0; i < n; i++)
        outputs[i] = garble_next_wire(ctxt);
    garble_gate_NOT(gc, ctxt, inputs[0], outputs[0]);
    int carry = inputs[0];
    for (uint64_t i = 1; i < n; i++) {
        uint64_t newCarry;
        garble_gate_XOR(gc, ctxt, inputs[i], carry, outputs[i]);
        newCarry = garble_next_wire(ctxt);
        garble_gate_AND(gc, ctxt, inputs[i], carry, newCarry);
        carry = newCarry;
    }
}

int SUBCircuit(garble_circuit *gc, garble_context *ctxt,
        uint64_t n, int *inputs, int *outputs) {
    int tempWires[n / 2];
    int tempWires2[n];
    int split = n / 2;
    NOTCircuit(gc, ctxt, n / 2, inputs + split, tempWires);
    INCCircuit(gc, ctxt, n / 2, tempWires, tempWires2 + split);
    memcpy(tempWires2, inputs, sizeof(int) * split);
    return ADDCircuit(gc, ctxt, n, tempWires2, outputs);
}

int SHLCircuit(garble_circuit *gc, garble_context *ctxt,
        uint64_t n, int *inputs, int *outputs) {
    outputs[0] = garble_gate_zero(gc, ctxt);
    memcpy(outputs + 1, inputs, sizeof(int) * (n - 1));
    return 0;
}

int SHRCircuit(garble_circuit *gc, garble_context *ctxt,
        uint64_t n, int *inputs, int *outputs) {
    outputs[n - 1] = garble_gate_zero(gc, ctxt);
    memcpy(outputs, inputs + 1, sizeof(int) * (n - 1));
    return 0;
}

int MULCircuit(garble_circuit *gc, garble_context *ctxt,
        uint64_t nt, int *inputs, int *outputs) {
    uint64_t n = nt / 2;
    int *A = inputs;
    int *B = inputs + n;

    int tempAnd[n][2 * n];
    int tempAddIn[4 * n];
    int tempAddOut[4 * n];

    for (uint64_t i = 0; i < n; i++) {
        for (uint64_t j = 0; j < i; j++) {
            tempAnd[i][j] = garble_gate_zero(gc, ctxt);
        }
        for (uint64_t j = i; j < i + n; j++) {
            tempAnd[i][j] = garble_next_wire(ctxt);
            garble_gate_AND(gc, ctxt, A[j - i], B[i], tempAnd[i][j]);
        }
        for (uint64_t j = i + n; j < 2 * n; j++)
            tempAnd[i][j] = garble_gate_zero(gc, ctxt);
    }

    for (uint64_t j = 0; j < 2 * n; j++) {
        tempAddOut[j] = tempAnd[0][j];
    }
    for (uint64_t i = 1; i < n; i++) {
        for (uint64_t j = 0; j < 2 * n; j++) {
            tempAddIn[j] = tempAddOut[j];
        }
        for (uint64_t j = 2 * n; j < 4 * n; j++) {
            tempAddIn[j] = tempAnd[i][j - 2 * n];
        }
        ADDCircuit(gc, ctxt, 4 * n, tempAddIn, tempAddOut);
    }
    for (uint64_t j = 0; j < 2 * n; j++) {
        outputs[j] = tempAddOut[j];
    }
    return 0;

}

void
GRECircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs)
{
    int tempWires[n];
    for (uint64_t i = 0; i < n / 2; i++) {
        tempWires[i] = inputs[i + n / 2];
        tempWires[i + n / 2] = inputs[i];
    }
    LESCircuit(gc, ctxt, n, tempWires, outputs);
}

int
MINCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           int *inputs, int *outputs)
{
    int i;
    int lesOutput;
    uint64_t notOutput = garble_next_wire(ctxt);
    LESCircuit(gc, ctxt, n, inputs, &lesOutput);
    garble_gate_NOT(gc, ctxt, lesOutput, notOutput);
    int split = n / 2;
    for (i = 0; i < split; i++)
        MUX21Circuit(gc, ctxt, lesOutput, inputs[i], inputs[split + i], outputs+i);
    return 0;
}

int LEQCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
        int *inputs, int *outputs) {
    int tempWires;
    GRECircuit(gc, ctxt, n, inputs, &tempWires);
    int outWire = garble_next_wire(ctxt);
    garble_gate_NOT(gc, ctxt, tempWires, outWire);
    outputs[0] = outWire;
    return 0;
}

int GEQCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
        int *inputs, int *outputs) {
    int tempWires;
    LESCircuit(gc, ctxt, n, inputs, &tempWires);
    int outWire = garble_next_wire(ctxt);
    garble_gate_NOT(gc, ctxt, tempWires, outWire);
    outputs[0] = outWire;
    return 0;
}

void
LESCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs)
{
    /* Returns 0 if the first number is less.
     * Returns 1 if the second number is less.
     * Returns 0 if the numbers are equal.
     */
    assert(n < 22); /* Tests fail for n >= 22 */
    assert(n % 2 == 0);

    int split = n/2;
    int **andInputs = malloc(sizeof(int*) * (split - 1));
    for (int i = 0; i < split - 1; i++) {
        andInputs[i] = malloc(sizeof(int) * (split - i));
        assert(andInputs[i]);
    }
    int *finalORInputs = malloc(sizeof(int) * split);
    /* Go bit by bit and those operations */
    for (int i = 0; i < split; i++) {
        int A = inputs[split + i];
        int B = inputs[i];

        uint64_t notA = garble_next_wire(ctxt);
        garble_gate_NOT(gc, ctxt, A, notA);

        uint64_t notB = garble_next_wire(ctxt);
        garble_gate_NOT(gc, ctxt, B, notB);

        int case1 = garble_next_wire(ctxt);
        garble_gate_AND(gc, ctxt, notA, B, case1);

        int case2 = garble_next_wire(ctxt);
        garble_gate_AND(gc, ctxt, A, notB, case2);

        if (i != split - 1)
            andInputs[i][0] = case1;

        int orOutput = garble_next_wire(ctxt);
        garble_gate_OR(gc, ctxt, case1, case2, orOutput);

        uint64_t norOutput = garble_next_wire(ctxt);
        garble_gate_NOT(gc, ctxt, orOutput, norOutput);

        for (int j = 0; j < i; j++) {
            andInputs[j][i-j] = norOutput;
        }
        if (i == split - 1)
            finalORInputs[split - 1] = case1;
    }

    /*  Do the aggregate operations with orInputs */
    for (int i = 0; i < split - 1; i++) {
        uint64_t nAndInputs = split - i;
        ANDCircuit(gc, ctxt, nAndInputs, andInputs[i], &finalORInputs[i]);
    }

    /* Final OR Circuit  */
    if (split == 1) {
        outputs[0] = finalORInputs[0];
    } else {
        ORCircuit(gc, ctxt, split, finalORInputs, outputs);
    }
    for (int i = 0; i < split - 1; i++)
        free(andInputs[i]);
    free(andInputs);
    free(finalORInputs);
}

int EQUCircuit(garble_circuit *gc, garble_context *ctxt, uint64_t n,
        int *inputs, int *outputs) {
    int tempWires[n / 2];

    XORCircuit(gc, ctxt, n, inputs, tempWires);
    int tempWire1 = tempWires[0];
    int tempWire2;
    for (uint64_t i = 1; i < n / 2; i++) {
        tempWire2 = garble_next_wire(ctxt);
        garble_gate_OR(gc, ctxt, tempWire1, tempWires[i], tempWire2);
        tempWire1 = tempWire2;
    }
    int outWire = garble_next_wire(ctxt);

    garble_gate_NOT(gc, ctxt, tempWire1, outWire);
    outputs[0] = outWire;
    return 0;
}


int ADDCircuit(garble_circuit *gc, garble_context *ctxt,
        uint64_t n, int *inputs, int *outputs) {
    int i;
    int tempOut[3];
    int split = n / 2;
    int tempIn[3];

    tempIn[0] = inputs[0];
    tempIn[1] = inputs[split];
    ADD22Circuit(gc, ctxt, tempIn, tempOut);
    outputs[0] = tempOut[0];

    for (i = 1; i < split; i++) {
        tempIn[2] = tempOut[1];
        tempIn[1] = inputs[split + i];
        tempIn[0] = inputs[i];
        ADD32Circuit(gc, ctxt, tempIn, tempOut);
        outputs[i] = tempOut[0];
    }
    return 0;
}

int ADD32Circuit(garble_circuit *gc,
                 garble_context *ctxt, int *inputs, int *outputs) {
    int wire1 = garble_next_wire(ctxt);
    int wire2 = garble_next_wire(ctxt);
    int wire3 = garble_next_wire(ctxt);
    int wire4 = garble_next_wire(ctxt);
    int wire5 = garble_next_wire(ctxt);

    garble_gate_XOR(gc, ctxt, inputs[2], inputs[0], wire1);
    garble_gate_XOR(gc, ctxt, inputs[1], inputs[0], wire2);
    garble_gate_XOR(gc, ctxt, inputs[2], wire2, wire3);
    garble_gate_AND(gc, ctxt, wire1, wire2, wire4);
    garble_gate_XOR(gc, ctxt, inputs[0], wire4, wire5);
    outputs[0] = wire3;
    outputs[1] = wire5;
    return 0;
}

int
ADD22Circuit(garble_circuit *gc, garble_context *ctxt,
             int *inputs, int *outputs)
{
    int wire1 = garble_next_wire(ctxt);
    int wire2 = garble_next_wire(ctxt);

    garble_gate_XOR(gc, ctxt, inputs[0], inputs[1], wire1);
    garble_gate_AND(gc, ctxt, inputs[0], inputs[1], wire2);
    outputs[0] = wire1;
    outputs[1] = wire2;
    return 0;
}
