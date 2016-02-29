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
} garble_type_e;

typedef enum {
    GARBLE_GATE_ZERO = 0,
    GARBLE_GATE_ONE = 15,
    GARBLE_GATE_AND = 8,
    GARBLE_GATE_OR = 14,
    GARBLE_GATE_XOR = 6,
    GARBLE_GATE_NOT = 5,
    GARBLE_GATE_EMPTY = -1,
} garble_gate_type_e;


typedef struct {
	block label0, label1;
} garble_wire;

typedef struct {
    garble_gate_type_e type;
	long input0, input1, output;
} garble_gate;

typedef struct {
	block table[3];
} garble_table;

typedef enum {
    GARBLE_FIXED_WIRE_ZERO = 0,
    GARBLE_FIXED_WIRE_ONE = 1
} garble_fixed_wire_e;

typedef struct {
    garble_fixed_wire_e type;
    int idx;
} garble_fixed_wire;

typedef struct {
	int n, m, q, r, nFixedWires;
	garble_gate *gates;            /* q */
	garble_table *garbledTable;    /* q */
	garble_wire *wires;            /* r */
    garble_fixed_wire *fixedWires; /* <= r */
	int *outputs;                  /* m */
    block fixedLabel;
	block globalKey;
} garble_circuit;

/* Used for constructing a circuit, and not for garbling as the name suggests */
typedef struct {
	long wireIndex;
} garble_context;

int
garble_garble(garble_circuit *gc, const block *inputLabels, block *outputLabels,
              garble_type_e type);
void
garble_hash(const garble_circuit *gc,
            unsigned char hash[SHA_DIGEST_LENGTH],
            garble_type_e type);
int
garble_check(garble_circuit *gc,
             const unsigned char hash[SHA_DIGEST_LENGTH],
             garble_type_e type);
block
garble_create_delta(void);
void
garble_create_input_labels(block *labels, int n, block *delta);

int
garble_eval(const garble_circuit *gc, const block *inputs, block *outputs,
            garble_type_e type);
void
garble_extract_labels(block *extracted, const block *labels, const int *bits,
                      long n);
int
garble_map_outputs(const block *outputs, const block *map, int *vals, int m);

int
garble_to_file(garble_circuit *gc, char *fname);
int
garble_from_file(garble_circuit *gc, char *fname);

int
garble_next_wire(garble_context *ctxt);

void
garble_start_building(garble_circuit *gc, garble_context *ctxt);
void
garble_finish_building(garble_circuit *gc, const int *outputs);

int
garble_new(garble_circuit *gc, int n, int m, int q, int r);
void
garble_delete(garble_circuit *gc);

size_t
garble_size(const garble_circuit *gc, bool wires);

int
garble_save(const garble_circuit *gc, FILE *f, bool wires);

int
garble_load(garble_circuit *gc, FILE *f, bool wires);

void
garble_to_buffer(const garble_circuit *gc, char *buf, bool wires);

int
garble_from_buffer(garble_circuit *gc, const char *buf, bool wires);

#endif
