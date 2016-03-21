#ifndef LIBGARBLE_H
#define LIBGARBLE_H

#include "garble/block.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <openssl/sha.h>

#define GARBLE_OK 0
#define GARBLE_ERR -1

/* Supported garbling types */
typedef enum {
    /* GRR3 and free-XOR as used in JustGarble */
    GARBLE_TYPE_STANDARD,
    /* Half-gates approach of Zahur, Rosulek, and Evans (Eurocrypt 2015) */
    GARBLE_TYPE_HALFGATES,
    /* Privacy-free approach of Zahur, Rosulek, and Evans (Eurocrypt 2015) */
    GARBLE_TYPE_PRIVACY_FREE,
} garble_type_e;

/* Supported gate types */
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
    /* The type of gate this is */
    garble_gate_type_e type;
    /* The input/output wires associated with this gate */
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
    /* n: number of inputs */
    /* m: number of outputs */
    /* q: number of gates */
    /* r: number of wires */
    /* n_fixed_wires: number of fixed wires */
    uint64_t n, m, q, r, n_fixed_wires;
    /* garbling scheme type */
    garble_type_e type;
    garble_gate *gates;         /* q */
    block *table;               /* q */
    block *wires;               /* 2 * r */
    garble_fixed_wire *fixed_wires; /* n_fixed_wires */
    int *outputs;                   /* m */
    block fixed_label;
    block global_key;
} garble_circuit;

/* Return the table size of a garbled circuit */
inline
size_t garble_table_size(const garble_circuit *gc)
{
    switch(gc->type) {
    case GARBLE_TYPE_STANDARD:
        return 3 * sizeof(block);
    case GARBLE_TYPE_HALFGATES:
        return 2 * sizeof(block);
    case GARBLE_TYPE_PRIVACY_FREE:
        return sizeof(block);
    }
    return 0;
}

/* Context info for building a circuit description */
typedef struct {
    uint64_t wire_index;
    uint64_t n_fixed_wires;
    uint64_t n_gates;
} garble_context;

/* Create a new circuit */
int
garble_new(garble_circuit *gc, uint64_t n, uint64_t m, garble_type_e type);
/* Delete a garbled circuit */
void
garble_delete(garble_circuit *gc);

/* Call this before constructing a new circuit */
void
garble_start_building(garble_circuit *gc, garble_context *ctxt);
/* Call this after completing the construction of a circuit */
void
garble_finish_building(garble_circuit *gc, garble_context *ctxt,
                       const int *outputs);

/* Garbles a circuit.
   If inputs is NULL, generate input-wire labels.
   If outputs is NULL, don't store output-wire labels into outputs.
 */
int
garble_garble(garble_circuit *gc, const block *inputs, block *outputs);
/* Hash a given garbled circuit */
void
garble_hash(const garble_circuit *gc, unsigned char hash[SHA_DIGEST_LENGTH]);
/* Check that 'gc' matches the hash specified in 'hash' */
int
garble_check(garble_circuit *gc, const unsigned char hash[SHA_DIGEST_LENGTH]);
/* Create a random delta block */
block
garble_create_delta(void);
/* Create 'n' input labels and store them in 'labels'.
   'delta' specified the delta block to use; if NULL, generate a new delta.
   'privacyfree' specifies whether you are using the privacy free garbling scheme.
 */
void
garble_create_input_labels(block *labels, uint64_t n, block *delta,
                           bool privacyfree);
/* Evaluate garbled circuit 'gc'. */
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
