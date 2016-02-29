#include "garble.h"

#include <string.h>

int
garble_next_wire(garble_context *ctxt)
{
    return ctxt->wireIndex++;
}

int
garble_new(garble_circuit *gc, int n, int m, int q, int r)
{
    gc->gates = calloc(q, sizeof(garble_gate));
    gc->fixedWires = calloc(r, sizeof(int));
    gc->outputs = calloc(m, sizeof(int));
    gc->wires = calloc(r, sizeof(garble_wire));
    gc->garbledTable = calloc(q, sizeof(garble_table));

	if (gc->gates == NULL || gc->fixedWires == NULL || gc->outputs == NULL
        || gc->wires == NULL || gc->garbledTable == NULL) {
        return -1;
	}
	gc->m = m;
	gc->n = n;
    /* q is set in finishBuilding() */
	gc->q = 0;
	gc->r = r;
    gc->nFixedWires = 0;
    return 0;
}

void
garble_delete(garble_circuit *gc)
{
	free(gc->gates);
    if (gc->garbledTable)
        free(gc->garbledTable);
    if (gc->wires)
        free(gc->wires);
    free(gc->fixedWires);
    free(gc->outputs);
}

void
garble_start_building(garble_circuit *gc, garble_context *ctxt)
{
    ctxt->wireIndex = gc->n; /* start at first non-input wire */
}

void
garble_finish_building(garble_circuit *gc, const int *outputs)
{
	for (int i = 0; i < gc->m; ++i) {
		gc->outputs[i] = outputs[i];
	}
}

size_t
garble_size(const garble_circuit *gc, bool wires)
{
    size_t size = 0;

    size += sizeof(int) * 5;
    size += sizeof(garble_gate) * gc->q;
    size += sizeof(garble_table) * gc->q;
    if (wires) {
        size += sizeof(garble_wire) * gc->r;
    }
    size += sizeof(garble_fixed_wire) * gc->nFixedWires;
    size += sizeof(int) * gc->m;
    size += sizeof(block);
    size += sizeof(block);

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
    p += cpy_to_buf(buf + p, &gc->nFixedWires, sizeof gc->nFixedWires);
    p += cpy_to_buf(buf + p, gc->gates, sizeof(garble_gate) * gc->q);
    p += cpy_to_buf(buf + p, gc->garbledTable, sizeof(garble_table) * gc->q);
    if (wires) {
        p += cpy_to_buf(buf + p, gc->wires, sizeof(garble_wire) * gc->r);
    }
    if (gc->nFixedWires > 0) {
        p += cpy_to_buf(buf + p, gc->fixedWires, sizeof(garble_fixed_wire) * gc->nFixedWires);
    }
    p += cpy_to_buf(buf + p, gc->outputs, sizeof(int) * gc->m);
    p += cpy_to_buf(buf + p, &gc->fixedLabel, sizeof(block));
    p += cpy_to_buf(buf + p, &gc->globalKey, sizeof(block));
}

int
garble_from_buffer(garble_circuit *gc, const char *buf, bool wires)
{
    size_t p = 0;
    p += cpy_to_buf(&gc->n, buf + p, sizeof gc->n);
    p += cpy_to_buf(&gc->m, buf + p, sizeof gc->m);
    p += cpy_to_buf(&gc->q, buf + p, sizeof gc->q);
    p += cpy_to_buf(&gc->r, buf + p, sizeof gc->r);
    p += cpy_to_buf(&gc->nFixedWires, buf + p, sizeof gc->nFixedWires);

    if ((gc->gates = malloc(sizeof(garble_gate) * gc->q)) == NULL)
        return -1;
    p += cpy_to_buf(gc->gates, buf + p, sizeof(garble_gate) * gc->q);

    gc->garbledTable = malloc(sizeof(garble_table) * gc->q);
    p += cpy_to_buf(gc->garbledTable, buf + p, sizeof(garble_table) * gc->q);

    if (wires) {
        gc->wires = calloc(gc->r, sizeof(garble_wire));
        p += cpy_to_buf(gc->wires, buf + p, sizeof(garble_wire) * gc->r);
    } else {
        gc->wires = NULL;
    }

    if (gc->nFixedWires) {
        gc->fixedWires = malloc(sizeof(garble_fixed_wire) * gc->nFixedWires);
        p += cpy_to_buf(gc->fixedWires, buf + p, sizeof(garble_fixed_wire) * gc->nFixedWires);
    } else {
        gc->fixedWires = NULL;
    }

    gc->outputs = malloc(sizeof(int) * gc->m);
    p += cpy_to_buf(gc->outputs, buf + p, sizeof(int) * gc->m);

    p += cpy_to_buf(&gc->fixedLabel, buf + p, sizeof(block));
    p += cpy_to_buf(&gc->globalKey, buf + p, sizeof(block));
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
    p += fread(&gc->nFixedWires, sizeof gc->nFixedWires, 1, f);

    if ((gc->gates = malloc(sizeof(garble_gate) * gc->q)) == NULL)
        return -1;
    p += fread(gc->gates, sizeof(garble_gate), gc->q, f);

    gc->garbledTable = malloc(sizeof(garble_table) * gc->q);
    p += fread(gc->garbledTable, sizeof(garble_table), gc->q, f);

    if (wires) {
        gc->wires = calloc(gc->r, sizeof(garble_wire));
        p += fread(gc->wires, sizeof(garble_wire), gc->r, f);
    } else {
        gc->wires = NULL;
    }

    if (gc->nFixedWires) {
        gc->fixedWires = calloc(gc->nFixedWires, sizeof(garble_fixed_wire));
        p += fread(gc->fixedWires, sizeof(garble_fixed_wire), gc->nFixedWires, f);
    } else {
        gc->fixedWires = NULL;
    }

    gc->outputs = malloc(sizeof(int) * gc->m);
    p += fread(gc->outputs, sizeof(int), gc->m, f);

    p += fread(&gc->fixedLabel, sizeof(block), 1, f);
    p += fread(&gc->globalKey, sizeof(block), 1, f);
    return 0;                   /* TODO: check mallocs */
}
