#include <garble.h>
#include "gates.h"
#include "circuits.h"

#include <assert.h>
#include <string.h>

void
circuit_and(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs)
{
    assert(n >= 2);

    outputs[0] = garble_next_wire(ctxt);
    gate_AND(gc, ctxt, inputs[0], inputs[1], outputs[0]);
    if (n > 2) {
        for (uint64_t i = 2; i < n; ++i) {
            int wire = garble_next_wire(ctxt);
            gate_AND(gc, ctxt, inputs[i], outputs[0], wire);
            outputs[0] = wire;
        }
    }
}

void
circuit_or(garble_circuit *gc, garble_context *ctxt, uint64_t n,
           const int *inputs, int *outputs)
{
    assert(n >= 2);

    outputs[0] = garble_next_wire(ctxt);
    gate_OR(gc, ctxt, inputs[0], inputs[1], outputs[0]);
    if (n > 2) {
        for (uint64_t i = 2; i < n; ++i) {
            int wire = garble_next_wire(ctxt);
            gate_OR(gc, ctxt, inputs[i], outputs[0], wire);
            outputs[0] = wire;
        }
    }
}

void
circuit_xor(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs)
{
    assert(n >= 2 && n % 2 == 0);
    for (uint64_t i = 0; i < n / 2; ++i) {
        int wire = garble_next_wire(ctxt);
        gate_XOR(gc, ctxt, inputs[i], inputs[n / 2 + i], wire);
        outputs[i] = wire;
    }
}

void
circuit_not(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs)
{
    for (uint64_t i = 0; i < n; ++i) {
        outputs[i] = garble_next_wire(ctxt);
        gate_NOT(gc, ctxt, inputs[i], outputs[i]);
    }
}

void
circuit_mixed(garble_circuit *gc, garble_context *ctxt, uint64_t n,
              const int *inputs, int outputs[1])
{
    int oldInternalWire = inputs[0];
    uint64_t newInternalWire;

    for (uint64_t i = 0; i < n - 1; i++) {
        newInternalWire = garble_next_wire(ctxt);
        if (i % 3 == 2)
            gate_OR(gc, ctxt, inputs[i + 1], oldInternalWire, newInternalWire);
        if (i % 3 == 1)
            gate_AND(gc, ctxt, inputs[i + 1], oldInternalWire, newInternalWire);
        if (i % 3 == 0)
            gate_XOR(gc, ctxt, inputs[i + 1], oldInternalWire, newInternalWire);
        oldInternalWire = newInternalWire;
    }
    outputs[0] = oldInternalWire;
}

void
circuit_multi_xor(garble_circuit *gc, garble_context *ctxt, int d,
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
        circuit_xor(gc, ctxt, 2 * div, tempInWires, tempOutWires);
    }
    for (int i = 0; i < div; i++) {
        outputs[i] = tempOutWires[i];
    }

    free(tempInWires);
    free(tempOutWires);
}

void
circuit_mux21(garble_circuit *gc, garble_context *ctxt, 
              int theSwitch, int input0, int input1, int output[1])
{
    uint64_t notSwitch = garble_next_wire(ctxt);
    gate_NOT(gc, ctxt, theSwitch, notSwitch);
    int and0 = garble_next_wire(ctxt);
    gate_AND(gc, ctxt, notSwitch, input0, and0);
    int and1 = garble_next_wire(ctxt);
    gate_AND(gc, ctxt, theSwitch, input1, and1);
    *output = garble_next_wire(ctxt);
    gate_OR(gc, ctxt, and0, and1, *output);
}

/* GF operations */

void
circuit_GF256_inv(garble_circuit *gc, garble_context *ctxt, const int inputs[8],
                  int outputs[8])
{
    int E[4], P[4], Q[4], tempX[4];
    int CD[8], EA[8], EB[8];

    circuit_xor(gc, ctxt, 8, inputs, tempX);
    circuit_GF16_sqcl(gc, ctxt, tempX, CD);
    circuit_GF16_mul(gc, ctxt, inputs, CD + 4);

    circuit_xor(gc, ctxt, 8, CD, tempX);
    circuit_GF16_inv(gc, ctxt, tempX, E);

    for (int i = 0; i < 4; i++) {
        EB[i] = E[i];
        EB[i + 4] = inputs[i + 4];
    }

    for (int i = 0; i < 4; i++) {
        EA[i] = E[i];
        EA[i + 4] = inputs[i];
    }

    circuit_GF16_mul(gc, ctxt, EB, P);
    circuit_GF16_mul(gc, ctxt, EA, Q);

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
circuit_GF16_inv(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
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

    circuit_xor(gc, ctxt, 4, ab, tempx);
    circuit_GF4_sq(tempx, tempxs);
    circuit_GF4_scln(gc, ctxt, tempxs, c);
    circuit_GF4_mul(gc, ctxt, ab, d);

    cd[0] = c[0];
    cd[1] = c[1];
    cd[2] = d[0];
    cd[3] = d[1];

    circuit_xor(gc, ctxt, 4, cd, tempx2);
    circuit_GF4_sq(tempx2, e);

    ea[0] = e[0];
    ea[1] = e[1];
    ea[2] = a[0];
    ea[3] = a[1];

    eb[0] = e[0];
    eb[1] = e[1];
    eb[2] = b[0];
    eb[3] = b[1];

    circuit_GF4_mul(gc, ctxt, eb, p);
    circuit_GF4_mul(gc, ctxt, ea, q);

    outputs[0] = q[0];
    outputs[1] = q[1];
    outputs[2] = p[0];
    outputs[3] = p[1];
}

void
circuit_GF16_mul(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
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

    circuit_xor(gc, ctxt, 4, ab, abcdx);
    circuit_xor(gc, ctxt, 4, cd, abcdx + 2);

    circuit_GF4_mul(gc, ctxt, abcdx, e);
    circuit_GF4_scln(gc, ctxt, e, em);

    ac[0] = ab[0];
    ac[1] = ab[1];
    ac[2] = cd[0];
    ac[3] = cd[1];

    bd[0] = ab[2 + 0];
    bd[1] = ab[2 + 1];
    bd[2] = cd[2 + 0];
    bd[3] = cd[2 + 1];

    circuit_GF4_mul(gc, ctxt, ac, tmpx1);
    circuit_GF4_mul(gc, ctxt, bd, tmpx2);

    tmpx1[2] = em[0];
    tmpx1[3] = em[1];
    tmpx2[2] = em[0];
    tmpx2[3] = em[1];

    circuit_xor(gc, ctxt, 4, tmpx1, p);
    circuit_xor(gc, ctxt, 4, tmpx2, q);

    outputs[0] = q[0];
    outputs[1] = q[1];
    outputs[2] = p[0];
    outputs[3] = p[1];
}

void
circuit_GF16_sqcl(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
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

    circuit_xor(gc, ctxt, 4, ab, tempx);

    circuit_GF4_sq(tempx, p);
    circuit_GF4_sq(b, tempx2);
    circuit_GF4_scln2(gc, ctxt, tempx2, q);

    outputs[0] = q[0];
    outputs[1] = q[1];
    outputs[2] = p[0];
    outputs[3] = p[1];
}

//http://edipermadi.files.wordpress.com/2008/02/aes_galois_field.jpg
void
circuit_GF8_mul(garble_circuit *gc, garble_context *ctxt, const int inputs[8],
                int outputs[8])
{
    outputs[0] = inputs[7];
    outputs[2] = inputs[1];
    outputs[3] = garble_next_wire(ctxt);
    gate_XOR(gc, ctxt, inputs[7], inputs[2], outputs[3]);

    outputs[4] = garble_next_wire(ctxt);
    gate_XOR(gc, ctxt, inputs[7], inputs[3], outputs[4]);

    outputs[5] = inputs[4];
    outputs[6] = inputs[5];
    outputs[7] = inputs[6];
    outputs[1] = garble_next_wire(ctxt);
    gate_XOR(gc, ctxt, inputs[7], inputs[0], outputs[1]);
}

void
circuit_GF4_mul(garble_circuit *gc, garble_context *ctxt, const int inputs[4],
                int outputs[2])
{
    int a, b, c, d, e, p, q, temp1, temp2;

    a = inputs[1];
    b = inputs[0];
    c = inputs[3];
    d = inputs[2];

    temp1 = garble_next_wire(ctxt);
    gate_XOR(gc, ctxt, a, b, temp1);
    temp2 = garble_next_wire(ctxt);
    gate_XOR(gc, ctxt, c, d, temp2);
    e = garble_next_wire(ctxt);
    gate_AND(gc, ctxt, temp1, temp2, e);
    temp1 = garble_next_wire(ctxt);
    gate_AND(gc, ctxt, a, c, temp1);
    p = garble_next_wire(ctxt);
    gate_XOR(gc, ctxt, temp1, e, p);
    temp2 = garble_next_wire(ctxt);
    gate_AND(gc, ctxt, b, d, temp2);
    q = garble_next_wire(ctxt);
    gate_XOR(gc, ctxt, temp2, e, q);

    outputs[1] = p;
    outputs[0] = q;
}

void
circuit_GF4_scln(garble_circuit *gc, garble_context *ctxt, const int inputs[2],
                 int outputs[2])
{
    outputs[0] = garble_next_wire(ctxt);
    gate_XOR(gc, ctxt, inputs[0], inputs[1], outputs[0]);
    outputs[1] = inputs[0];
}

void
circuit_GF4_scln2(garble_circuit *gc, garble_context *ctxt, const int inputs[2],
                  int outputs[2])
{
    outputs[1] = garble_next_wire(ctxt);
    gate_XOR(gc, ctxt, inputs[0], inputs[1], outputs[1]);
    outputs[0] = inputs[1];
}

void
circuit_GF4_sq(const int inputs[2], int outputs[2])
{
    outputs[0] = inputs[1];
    outputs[1] = inputs[0];
}

void
circuit_rand(garble_circuit *gc, garble_context *ctxt, uint64_t n,
             int outputs[1], int q, int qf)
{
    int oldInternalWire = garble_next_wire(ctxt);
    uint64_t newInternalWire;

    gate_AND(gc, ctxt, 0, 1, oldInternalWire);

    for (int i = 2; i < q + qf - 1; i++) {
        newInternalWire = garble_next_wire(ctxt);
        if (i < q)
            gate_AND(gc, ctxt, i % n, oldInternalWire, newInternalWire);
        else
            gate_XOR(gc, ctxt, i % n, oldInternalWire, newInternalWire);
        oldInternalWire = newInternalWire;
    }
    outputs[0] = oldInternalWire;
}

void
circuit_inc(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs)
{
    for (uint64_t i = 0; i < n; i++)
        outputs[i] = garble_next_wire(ctxt);
    gate_NOT(gc, ctxt, inputs[0], outputs[0]);
    int carry = inputs[0];
    for (uint64_t i = 1; i < n; i++) {
        uint64_t newCarry;
        gate_XOR(gc, ctxt, inputs[i], carry, outputs[i]);
        newCarry = garble_next_wire(ctxt);
        gate_AND(gc, ctxt, inputs[i], carry, newCarry);
        carry = newCarry;
    }
}

void
circuit_sub(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs)
{
    int tempWires[n / 2];
    int tempWires2[n];
    int split = n / 2;
    circuit_not(gc, ctxt, n / 2, inputs + split, tempWires);
    circuit_inc(gc, ctxt, n / 2, tempWires, tempWires2 + split);
    memcpy(tempWires2, inputs, sizeof(int) * split);
    circuit_add(gc, ctxt, n, tempWires2, outputs, NULL);
}

void
circuit_shl(garble_circuit *gc, uint64_t n, const int *inputs, int *outputs)
{
    outputs[0] = wire_zero(gc);
    memcpy(outputs + 1, inputs, sizeof(int) * (n - 1));
}

void
circuit_shr(garble_circuit *gc, uint64_t n, const int *inputs, int *outputs)
{
    outputs[n - 1] = wire_zero(gc);
    memcpy(outputs, inputs + 1, sizeof(int) * (n - 1));
}

void
circuit_mul(garble_circuit *gc, garble_context *ctxt, uint64_t nt,
            int *inputs, int *outputs)
{
    uint64_t n = nt / 2;
    int *A = inputs;
    int *B = inputs + n;

    int tempAnd[n][2 * n];
    int tempAddIn[4 * n];
    int tempAddOut[4 * n];

    for (uint64_t i = 0; i < n; i++) {
        for (uint64_t j = 0; j < i; j++) {
            tempAnd[i][j] = wire_zero(gc);
        }
        for (uint64_t j = i; j < i + n; j++) {
            tempAnd[i][j] = garble_next_wire(ctxt);
            gate_AND(gc, ctxt, A[j - i], B[i], tempAnd[i][j]);
        }
        for (uint64_t j = i + n; j < 2 * n; j++)
            tempAnd[i][j] = wire_zero(gc);
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
        circuit_add(gc, ctxt, 4 * n, tempAddIn, tempAddOut, NULL);
    }
    for (uint64_t j = 0; j < 2 * n; j++) {
        outputs[j] = tempAddOut[j];
    }
}

void
circuit_gre(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs)
{
    int tempWires[n];
    for (uint64_t i = 0; i < n / 2; i++) {
        tempWires[i] = inputs[i + n / 2];
        tempWires[i + n / 2] = inputs[i];
    }
    circuit_les(gc, ctxt, n, tempWires, outputs);
}

void
circuit_min(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            int *inputs, int *outputs)
{
    int i;
    int lesOutput;
    uint64_t notOutput = garble_next_wire(ctxt);
    circuit_les(gc, ctxt, n, inputs, &lesOutput);
    gate_NOT(gc, ctxt, lesOutput, notOutput);
    int split = n / 2;
    for (i = 0; i < split; i++)
        circuit_mux21(gc, ctxt, lesOutput, inputs[i], inputs[split + i], outputs+i);
}

void
circuit_leq(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs)
{
    int tempWires;
    circuit_gre(gc, ctxt, n, inputs, &tempWires);
    int outWire = garble_next_wire(ctxt);
    gate_NOT(gc, ctxt, tempWires, outWire);
    outputs[0] = outWire;
}

void
circuit_geq(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs)
{
    int tempWires;
    circuit_les(gc, ctxt, n, inputs, &tempWires);
    int outWire = garble_next_wire(ctxt);
    gate_NOT(gc, ctxt, tempWires, outWire);
    outputs[0] = outWire;
}

void
circuit_les(garble_circuit *gc, garble_context *ctxt, uint64_t n,
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
        gate_NOT(gc, ctxt, A, notA);

        uint64_t notB = garble_next_wire(ctxt);
        gate_NOT(gc, ctxt, B, notB);

        int case1 = garble_next_wire(ctxt);
        gate_AND(gc, ctxt, notA, B, case1);

        int case2 = garble_next_wire(ctxt);
        gate_AND(gc, ctxt, A, notB, case2);

        if (i != split - 1)
            andInputs[i][0] = case1;

        int orOutput = garble_next_wire(ctxt);
        gate_OR(gc, ctxt, case1, case2, orOutput);

        uint64_t norOutput = garble_next_wire(ctxt);
        gate_NOT(gc, ctxt, orOutput, norOutput);

        for (int j = 0; j < i; j++) {
            andInputs[j][i-j] = norOutput;
        }
        if (i == split - 1)
            finalORInputs[split - 1] = case1;
    }

    /*  Do the aggregate operations with orInputs */
    for (int i = 0; i < split - 1; i++) {
        uint64_t nAndInputs = split - i;
        circuit_and(gc, ctxt, nAndInputs, andInputs[i], &finalORInputs[i]);
    }

    /* Final OR Circuit  */
    if (split == 1) {
        outputs[0] = finalORInputs[0];
    } else {
        circuit_or(gc, ctxt, split, finalORInputs, outputs);
    }
    for (int i = 0; i < split - 1; i++)
        free(andInputs[i]);
    free(andInputs);
    free(finalORInputs);
}

/* Check equality of two n/2-bit values */
void
circuit_equ(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int outputs[1])
{
    int tempWire1, tempWire2, outWire;
    int *tempWires;
    uint64_t split = n / 2;

    assert(n % 2 == 0 && n >= 2);

    tempWires = calloc(split, sizeof(int));

    circuit_xor(gc, ctxt, n, inputs, tempWires);
    tempWire1 = tempWires[0];
    for (uint64_t i = 1; i < split; ++i) {
        tempWire2 = garble_next_wire(ctxt);
        gate_OR(gc, ctxt, tempWire1, tempWires[i], tempWire2);
        tempWire1 = tempWire2;
    }
    outWire = garble_next_wire(ctxt);

    gate_NOT(gc, ctxt, tempWire1, outWire);
    outputs[0] = outWire;

    free(tempWires);
}

/* Add two n/2-bit values together */
void
circuit_add(garble_circuit *gc, garble_context *ctxt, uint64_t n,
            const int *inputs, int *outputs, int *carry)
{
    int tempIn[3], tempOut[2];
    int split = n / 2;

    assert(n % 2 == 0 && n >= 2);

    tempIn[0] = inputs[0];
    tempIn[1] = inputs[split];
    circuit_add22(gc, ctxt, tempIn, tempOut);
    outputs[0] = tempOut[0];

    for (int i = 1; i < split; ++i) {
        tempIn[0] = inputs[i];
        tempIn[1] = inputs[split + i];
        tempIn[2] = tempOut[1];
        circuit_add32(gc, ctxt, tempIn, tempOut);
        outputs[i] = tempOut[0];
    }
    if (carry)
        *carry = tempOut[1];
}

/* Full adder: https://en.wikipedia.org/wiki/Adder_%28electronics%29#Full_adder */
void
circuit_add32(garble_circuit *gc, garble_context *ctxt, const int inputs[3],
              int outputs[2])
{
    int wire1 = garble_next_wire(ctxt);
    int wire2 = garble_next_wire(ctxt);
    int wire3 = garble_next_wire(ctxt);
    int wire4 = garble_next_wire(ctxt);
    int wire5 = garble_next_wire(ctxt);

    gate_XOR(gc, ctxt, inputs[2], inputs[0], wire1);
    gate_XOR(gc, ctxt, inputs[1], inputs[0], wire2);
    gate_XOR(gc, ctxt, inputs[2], wire2, wire3);
    gate_AND(gc, ctxt, wire1, wire2, wire4);
    gate_XOR(gc, ctxt, inputs[0], wire4, wire5);
    outputs[0] = wire3;         /* Sum */
    outputs[1] = wire5;         /* Carry */
}

/* Half adder: https://en.wikipedia.org/wiki/Adder_%28electronics%29#Half_adder */
void
circuit_add22(garble_circuit *gc, garble_context *ctxt, const int inputs[2],
              int outputs[2])
{
    int wire1 = garble_next_wire(ctxt);
    int wire2 = garble_next_wire(ctxt);

    gate_XOR(gc, ctxt, inputs[0], inputs[1], wire1);
    gate_AND(gc, ctxt, inputs[0], inputs[1], wire2);
    outputs[0] = wire1;         /* Sum */
    outputs[1] = wire2;         /* Carry */
}
