/*
 This file is part of LibGarble, based on JustGarble.
*/
#ifndef LIBGARBLE_H
#define LIBGARBLE_H

#include "garble/block.h"

#include <stdbool.h>
#include <stdint.h>
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
	uint64_t input0, input1, output;
} garble_gate;

typedef enum {
    GARBLE_FIXED_WIRE_ZERO = 0,
    GARBLE_FIXED_WIRE_ONE = 1
} garble_fixed_wire_e;

typedef struct {
    garble_fixed_wire_e type;
    int idx;
} garble_fixed_wire;

typedef struct {
    uint64_t n, m, q, r, n_fixed_wires;
    garble_type_e type;
    garble_gate *gates;         /* q */
    block *table;               /* q */
    garble_wire *wires;         /* r */
    garble_fixed_wire *fixed_wires; /* <= r */
    int *outputs;                   /* m */
    block fixed_label;
    block global_key;
} garble_circuit;

inline
size_t garble_table_size(const garble_circuit *gc)
{
    switch(gc->type) {
    case GARBLE_TYPE_STANDARD:
        return 3 * sizeof(block);
    case GARBLE_TYPE_HALFGATES:
        return 2 * sizeof(block);
    }
    return 0;
}

typedef struct {
	uint64_t wire_index;
} garble_context;

int
garble_new(garble_circuit *gc, uint64_t n, uint64_t m, uint64_t q, uint64_t r,
           garble_type_e type);
void
garble_delete(garble_circuit *gc);

void
garble_start_building(garble_circuit *gc, garble_context *ctxt);
void
garble_finish_building(garble_circuit *gc, const int *outputs);

int
garble_garble(garble_circuit *gc, const block *inputs, block *outputs);

void
garble_hash(const garble_circuit *gc, unsigned char hash[SHA_DIGEST_LENGTH]);
int
garble_check(garble_circuit *gc, const unsigned char hash[SHA_DIGEST_LENGTH]);
block
garble_create_delta(void);
void
garble_create_input_labels(block *labels, uint64_t n, block *delta);

int
garble_eval(const garble_circuit *gc, const block *inputs, block *outputs);
void
garble_extract_labels(block *extracted, const block *labels, const bool *bits,
                      uint64_t n);
int
garble_map_outputs(const block *outputs, const block *map, bool *vals,
                   uint64_t m);

/* write/read circuit description to/from file */
int
garble_circuit_to_file(garble_circuit *gc, char *fname);
int
garble_circuit_from_file(garble_circuit *gc, char *fname);

int
garble_next_wire(garble_context *ctxt);

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
