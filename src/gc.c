#include "garble.h"

#include <string.h>

extern inline
size_t garble_table_size(const garble_circuit *gc);

int
garble_next_wire(garble_context *ctxt)
{
    return ctxt->wire_index++;
}

int
garble_new(garble_circuit *gc, uint64_t n, uint64_t m, uint64_t q, uint64_t r,
           garble_type_e type)
{
    if (gc == NULL)
        return GARBLE_ERR;

    gc->gates = calloc(q, sizeof(garble_gate));
    gc->fixed_wires = calloc(r, sizeof(garble_fixed_wire));
    gc->outputs = calloc(m, sizeof(int));
    gc->wires = calloc(r, sizeof(garble_wire));
    gc->table = NULL;
    /* switch (type) { */
    /* case GARBLE_TYPE_STANDARD: */
    /*     gc->table = calloc(q, 3 * sizeof(block)); */
    /*     break; */
    /* case GARBLE_TYPE_HALFGATES: */
    /*     gc->table = calloc(q, 2 * sizeof(block)); */
    /*     break; */
    /* } */

    gc->type = type;
	gc->m = m;
	gc->n = n;
    /* q is set in garble_finish_building() */
	gc->q = 0;
	gc->r = r;
    gc->n_fixed_wires = 0;
    return GARBLE_OK;
}

void
garble_delete(garble_circuit *gc)
{
    if (gc == NULL)
        return;
	free(gc->gates);
    if (gc->table)
        free(gc->table);
    if (gc->wires)
        free(gc->wires);
    free(gc->fixed_wires);
    free(gc->outputs);
}

void
garble_start_building(garble_circuit *gc, garble_context *ctxt)
{
    ctxt->wire_index = gc->n; /* start at first non-input wire */
}

void
garble_finish_building(garble_circuit *gc, const int *outputs)
{
	for (uint64_t i = 0; i < gc->m; ++i) {
		gc->outputs[i] = outputs[i];
	}
}

size_t
garble_size(const garble_circuit *gc, bool wires)
{
    size_t size = 0;

    size += sizeof gc->n + sizeof gc->m + sizeof gc->q + sizeof gc->r
        + sizeof gc->n_fixed_wires;
    size += sizeof gc->type;
    size += sizeof(garble_gate) * gc->q;
    size += garble_table_size(gc) * gc->q;
    if (wires)
        size += sizeof(garble_wire) * gc->r;
    size += sizeof(garble_fixed_wire) * gc->n_fixed_wires;
    size += sizeof(int) * gc->m;
    size += sizeof gc->fixed_label;
    size += sizeof gc->global_key;

    return size;
}

inline static size_t
cpy_to_buf(void *out, const void *in, size_t size)
{
    (void) memcpy(out, in, size);
    return size;
}

void
garble_to_buffer(const garble_circuit *gc, char *buf, bool wires)
{
    size_t p = 0;
    p += cpy_to_buf(buf + p, &gc->n, sizeof gc->n);
    p += cpy_to_buf(buf + p, &gc->m, sizeof gc->m);
    p += cpy_to_buf(buf + p, &gc->q, sizeof gc->q);
    p += cpy_to_buf(buf + p, &gc->r, sizeof gc->r);
    p += cpy_to_buf(buf + p, &gc->n_fixed_wires, sizeof gc->n_fixed_wires);
    p += cpy_to_buf(buf + p, &gc->type, sizeof gc->type);

    p += cpy_to_buf(buf + p, gc->gates, sizeof(garble_gate) * gc->q);
    p += cpy_to_buf(buf + p, gc->table, garble_table_size(gc) * gc->q);
    if (wires)
        p += cpy_to_buf(buf + p, gc->wires, sizeof(garble_wire) * gc->r);
    if (gc->n_fixed_wires > 0) {
        p += cpy_to_buf(buf + p, gc->fixed_wires,
                        sizeof(garble_fixed_wire) * gc->n_fixed_wires);
    }
    p += cpy_to_buf(buf + p, gc->outputs, sizeof(int) * gc->m);
    p += cpy_to_buf(buf + p, &gc->fixed_label, sizeof(block));
    p += cpy_to_buf(buf + p, &gc->global_key, sizeof(block));
}

int
garble_from_buffer(garble_circuit *gc, const char *buf, bool wires)
{
    size_t p = 0;
    p += cpy_to_buf(&gc->n, buf + p, sizeof gc->n);
    p += cpy_to_buf(&gc->m, buf + p, sizeof gc->m);
    p += cpy_to_buf(&gc->q, buf + p, sizeof gc->q);
    p += cpy_to_buf(&gc->r, buf + p, sizeof gc->r);
    p += cpy_to_buf(&gc->n_fixed_wires, buf + p, sizeof gc->n_fixed_wires);
    p += cpy_to_buf(&gc->type, buf + p, sizeof gc->type);

    if ((gc->gates = malloc(sizeof(garble_gate) * gc->q)) == NULL)
        return -1;
    p += cpy_to_buf(gc->gates, buf + p, sizeof(garble_gate) * gc->q);

    gc->table = malloc(garble_table_size(gc) * gc->q);
    p += cpy_to_buf(gc->table, buf + p, garble_table_size(gc) * gc->q);

    if (wires) {
        gc->wires = calloc(gc->r, sizeof(garble_wire));
        p += cpy_to_buf(gc->wires, buf + p, sizeof(garble_wire) * gc->r);
    } else {
        gc->wires = NULL;
    }

    if (gc->n_fixed_wires) {
        gc->fixed_wires = malloc(sizeof(garble_fixed_wire) * gc->n_fixed_wires);
        p += cpy_to_buf(gc->fixed_wires, buf + p,
                        sizeof(garble_fixed_wire) * gc->n_fixed_wires);
    } else {
        gc->fixed_wires = NULL;
    }

    gc->outputs = malloc(sizeof(int) * gc->m);
    p += cpy_to_buf(gc->outputs, buf + p, sizeof(int) * gc->m);

    p += cpy_to_buf(&gc->fixed_label, buf + p, sizeof(block));
    p += cpy_to_buf(&gc->global_key, buf + p, sizeof(block));
    return 0;                   /* TODO: check mallocs */
}

int
garble_save(const garble_circuit *gc, FILE *f, bool wires)
{
    char *buf;
    size_t res, size = garble_size(gc, wires);

    if ((buf = malloc(size)) == NULL)
        return -1;
    garble_to_buffer(gc, buf, wires);
    res = fwrite(buf, sizeof(char), size, f);
    free(buf);
    return res == size ? 0 : -1;
}

int
garble_load(garble_circuit *gc, FILE *f, bool wires)
{
    size_t p = 0;

    p += fread(&gc->n, sizeof gc->n, 1, f);
    p += fread(&gc->m, sizeof gc->m, 1, f);
    p += fread(&gc->q, sizeof gc->q, 1, f);
    p += fread(&gc->r, sizeof gc->r, 1, f);
    p += fread(&gc->n_fixed_wires, sizeof gc->n_fixed_wires, 1, f);
    p += fread(&gc->type, sizeof gc->type, 1, f);

    if ((gc->gates = malloc(sizeof(garble_gate) * gc->q)) == NULL)
        return -1;
    p += fread(gc->gates, sizeof(garble_gate), gc->q, f);

    gc->table = malloc(garble_table_size(gc) * gc->q);
    p += fread(gc->table, garble_table_size(gc), gc->q, f);

    if (wires) {
        gc->wires = calloc(gc->r, sizeof(garble_wire));
        p += fread(gc->wires, sizeof(garble_wire), gc->r, f);
    } else {
        gc->wires = NULL;
    }

    if (gc->n_fixed_wires) {
        gc->fixed_wires = calloc(gc->n_fixed_wires, sizeof(garble_fixed_wire));
        p += fread(gc->fixed_wires, sizeof(garble_fixed_wire),
                   gc->n_fixed_wires, f);
    } else {
        gc->fixed_wires = NULL;
    }

    gc->outputs = malloc(sizeof(int) * gc->m);
    p += fread(gc->outputs, sizeof(int), gc->m, f);

    p += fread(&gc->fixed_label, sizeof(block), 1, f);
    p += fread(&gc->global_key, sizeof(block), 1, f);
    return 0;                   /* TODO: check mallocs */
}
