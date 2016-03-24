#include "garble.h"

#include <string.h>

int
garble_next_wire(garble_context *ctxt)
{
    return ctxt->wire_index++;
}

int
garble_new(garble_circuit *gc, uint64_t n, uint64_t m, garble_type_e type)
{
    if (gc == NULL)
        return GARBLE_ERR;

    gc->gates = NULL;
    gc->outputs = calloc(m, sizeof(int));
    gc->wires = NULL;
    gc->table = NULL;
    gc->output_perms = NULL;

    gc->type = type;
    gc->n = n;
    gc->m = m;
    /* q is incremented in circ/gates.c:_gate */
    gc->q = 0;
    /* r is set in garble_finish_building() */
    gc->r = 0;
    return GARBLE_OK;
}

void
garble_delete(garble_circuit *gc)
{
    if (gc == NULL)
        return;
    if (gc->gates)
        free(gc->gates);
    if (gc->table)
        free(gc->table);
    if (gc->wires)
        free(gc->wires);
    if (gc->outputs)
        free(gc->outputs);
    if (gc->output_perms)
        free(gc->output_perms);
}

void
garble_start_building(garble_circuit *gc, garble_context *ctxt)
{
    ctxt->wire_index = gc->n + 2; /* start at first non-input, non-fixed wire */
    ctxt->n_gates = 0;
}

void
garble_finish_building(garble_circuit *gc, garble_context *ctxt,
                       const int *outputs)
{
    gc->r = ctxt->wire_index + gc->q;
    for (uint64_t i = 0; i < gc->m; ++i) {
        gc->outputs[i] = outputs[i];
    }
}

size_t
garble_size(const garble_circuit *gc, bool wires)
{
    size_t size = 0;

    size += sizeof gc->n + sizeof gc->m + sizeof gc->q + sizeof gc->r;
    size += sizeof gc->type;
    size += sizeof(garble_gate) * gc->q;
    size += garble_table_size(gc) * gc->q;
    if (wires)
        size += sizeof(block) * 2 * gc->r;
    size += sizeof(int) * gc->m;
    size += sizeof(bool) * gc->m;
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

int
garble_to_buffer(const garble_circuit *gc, char *buf, bool wires)
{
    size_t p = 0;

    if (buf == NULL) {
        buf = malloc(garble_size(gc, wires));
        if (buf == NULL)
            return GARBLE_ERR;
    }
        
    p += cpy_to_buf(buf + p, &gc->n, sizeof gc->n);
    p += cpy_to_buf(buf + p, &gc->m, sizeof gc->m);
    p += cpy_to_buf(buf + p, &gc->q, sizeof gc->q);
    p += cpy_to_buf(buf + p, &gc->r, sizeof gc->r);
    p += cpy_to_buf(buf + p, &gc->type, sizeof gc->type);

    p += cpy_to_buf(buf + p, gc->gates, sizeof(garble_gate) * gc->q);
    p += cpy_to_buf(buf + p, gc->table, garble_table_size(gc) * gc->q);
    if (wires)
        p += cpy_to_buf(buf + p, gc->wires, sizeof(block) * 2 * gc->r);
    p += cpy_to_buf(buf + p, gc->outputs, sizeof(int) * gc->m);
    p += cpy_to_buf(buf + p, gc->outputs, sizeof(bool) * gc->m);
    p += cpy_to_buf(buf + p, &gc->fixed_label, sizeof(block));
    p += cpy_to_buf(buf + p, &gc->global_key, sizeof(block));

    return GARBLE_OK;
}

int
garble_from_buffer(garble_circuit *gc, const char *buf, bool wires)
{
    size_t p = 0;

    if (gc == NULL)
        return GARBLE_ERR;

    p += cpy_to_buf(&gc->n, buf + p, sizeof gc->n);
    p += cpy_to_buf(&gc->m, buf + p, sizeof gc->m);
    p += cpy_to_buf(&gc->q, buf + p, sizeof gc->q);
    p += cpy_to_buf(&gc->r, buf + p, sizeof gc->r);
    p += cpy_to_buf(&gc->type, buf + p, sizeof gc->type);

    if ((gc->gates = malloc(sizeof(garble_gate) * gc->q)) == NULL) {
        goto error;
    }
    p += cpy_to_buf(gc->gates, buf + p, sizeof(garble_gate) * gc->q);

    if ((gc->table = malloc(garble_table_size(gc) * gc->q)) == NULL) {
        goto error;
    }
        
    p += cpy_to_buf(gc->table, buf + p, garble_table_size(gc) * gc->q);

    if (wires) {
        if ((gc->wires = malloc(sizeof(block) * 2 * gc->r)) == NULL) {
            goto error;
        }
        p += cpy_to_buf(gc->wires, buf + p, sizeof(block) * 2 * gc->r);
    } else {
        gc->wires = NULL;
    }

    if ((gc->outputs = malloc(sizeof(int) * gc->m)) == NULL) {
        goto error;
    }
    p += cpy_to_buf(gc->outputs, buf + p, sizeof(int) * gc->m);
    if ((gc->output_perms = malloc(sizeof(bool) * gc->m)) == NULL) {
        goto error;
    }
    p += cpy_to_buf(gc->outputs, buf + p, sizeof(bool) * gc->m);

    p += cpy_to_buf(&gc->fixed_label, buf + p, sizeof(block));
    p += cpy_to_buf(&gc->global_key, buf + p, sizeof(block));

    return GARBLE_OK;

error:
    garble_delete(gc);
    return GARBLE_ERR;
}

int
garble_save(const garble_circuit *gc, FILE *f, bool wires)
{
    char *buf;
    size_t res, size = garble_size(gc, wires);

    if ((buf = malloc(size)) == NULL)
        return GARBLE_ERR;
    garble_to_buffer(gc, buf, wires);
    res = fwrite(buf, sizeof(char), size, f);
    free(buf);
    return res == size ? GARBLE_OK : GARBLE_ERR;
}

int
garble_load(garble_circuit *gc, FILE *f, bool wires)
{
    size_t p = 0;

    p += fread(&gc->n, sizeof gc->n, 1, f);
    p += fread(&gc->m, sizeof gc->m, 1, f);
    p += fread(&gc->q, sizeof gc->q, 1, f);
    p += fread(&gc->r, sizeof gc->r, 1, f);
    p += fread(&gc->type, sizeof gc->type, 1, f);

    if ((gc->gates = malloc(sizeof(garble_gate) * gc->q)) == NULL)
        goto error;
    p += fread(gc->gates, sizeof(garble_gate), gc->q, f);

    if ((gc->table = malloc(garble_table_size(gc) * gc->q)) == NULL)
        goto error;
    p += fread(gc->table, garble_table_size(gc), gc->q, f);

    if (wires) {
        if ((gc->wires = malloc(sizeof(block) * 2 * gc->r)) == NULL)
            goto error;
        p += fread(gc->wires, sizeof(block), 2 * gc->r, f);
    } else {
        gc->wires = NULL;
    }

    if ((gc->outputs = malloc(sizeof(int) * gc->m)) == NULL)
        goto error;
    p += fread(gc->outputs, sizeof(int), gc->m, f);
    if ((gc->output_perms = malloc(sizeof(bool) * gc->m)) == NULL)
        goto error;
    p += fread(gc->outputs, sizeof(bool), gc->m, f);

    p += fread(&gc->fixed_label, sizeof(block), 1, f);
    p += fread(&gc->global_key, sizeof(block), 1, f);
    return GARBLE_OK;

error:
    garble_delete(gc);
    return GARBLE_ERR;
}
