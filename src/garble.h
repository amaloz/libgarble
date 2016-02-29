/*
 This file is part of LibGarble, based on JustGarble.
*/

/*
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

#ifndef LIBGARBLE_H
#define LIBGARBLE_H

#include "garble/block.h"

#include <stdbool.h>
#include <stdio.h>
#include <openssl/sha.h>

#define GARBLE_OK 0
#define GARBLE_ERR -1

typedef enum {
    GARBLE_TYPE_STANDARD,
    GARBLE_TYPE_HALFGATES,
} GarbleType;

typedef struct {
	block label0, label1;
} Wire;

typedef struct {
    int type;
	long input0, input1, output;
} Gate;

typedef struct {
	block table[3];
} GarbledTable;

typedef enum { FIXED_WIRE_ZERO = 0, FIXED_WIRE_ONE = 1 } FixedWire_e;

typedef struct {
    FixedWire_e type;
    int idx;
} FixedWire;

typedef struct {
	int n, m, q, r, nFixedWires;
	Gate *gates;                /* q */
	GarbledTable *garbledTable; /* q */
	Wire *wires;                /* r */
    FixedWire *fixedWires;      /* <= r */
	int *outputs;               /* m */
    block fixedLabel;
	block globalKey;
} GarbledCircuit;

/* Used for constructing a circuit, and not for garbling as the name suggests */
typedef struct {
	long wireIndex;
} GarblingContext;

#define FIXED_ZERO_GATE 0
#define FIXED_ONE_GATE 15
#define ANDGATE 8
#define ORGATE 14
#define XORGATE 6
#define NOTGATE 5
#define NO_GATE -1

int
garble_garble(GarbledCircuit *gc, const block *inputLabels, block *outputLabels,
              GarbleType type);
void
garble_hash(const GarbledCircuit *gc,
            unsigned char hash[SHA_DIGEST_LENGTH],
            GarbleType type);
int
garble_check(GarbledCircuit *gc,
             const unsigned char hash[SHA_DIGEST_LENGTH],
             GarbleType type);
block
garble_create_delta(void);
void
garble_create_input_labels(block *labels, int n, block *delta);

int
garble_eval(const GarbledCircuit *gc, const block *inputs, block *outputs,
            GarbleType type);
void
garble_extract_labels(block *extracted, const block *labels, const int *bits,
                      long n);
int
garble_map_outputs(const block *outputs, const block *map, int *vals, int m);

int
garble_to_file(GarbledCircuit *gc, char *fname);
int
garble_from_file(GarbledCircuit *gc, char *fname);

int
garble_next_wire(GarblingContext *ctxt);

void
garble_start_building(GarbledCircuit *gc, GarblingContext *ctxt);
void
garble_finish_building(GarbledCircuit *gc, const int *outputs);

int
garble_new(GarbledCircuit *gc, int n, int m, int q, int r);
void
garble_delete(GarbledCircuit *gc);

size_t
garble_size(const GarbledCircuit *gc, bool wires);

int
garble_save(const GarbledCircuit *gc, FILE *f, bool wires);

int
garble_load(GarbledCircuit *gc, FILE *f, bool wires);

void
garble_to_buffer(const GarbledCircuit *gc, char *buf, bool wires);

int
garble_from_buffer(GarbledCircuit *gc, const char *buf, bool wires);

#endif
