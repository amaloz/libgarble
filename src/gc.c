#include "garble.h"

#include <assert.h>
#include <string.h>

int
garble_new(garble_circuit *gc, size_t n, size_t m, garble_type_e type)
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
    /* q, nxors incremented when building circuit */
    gc->q = 0;
    gc->nxors = 0;
    /* r set in garble_finish_building() */
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
garble_fprint(FILE *fp, garble_circuit *gc)
{
    fprintf(fp, "%lu %lu %lu (%lu) %lu\n", gc->n, gc->m, gc->q, gc->nxors, gc->r);
    block_fprintf(fp, "%B %B\n", gc->fixed_label, gc->global_key);
}

size_t
garble_size(const garble_circuit *gc, bool table_only, bool wires)
{
    size_t size = 0;
    if (table_only) {
        size += sizeof gc->q + sizeof gc->nxors;
        size += garble_table_size(gc) * (gc->q - gc->nxors);
        size += sizeof gc->fixed_label;
        size += sizeof gc->global_key;
        size += sizeof(bool) * gc->m;
    } else {
        size += sizeof gc->n + sizeof gc->m + sizeof gc->q + sizeof gc->r + sizeof gc->nxors;
        size += sizeof gc->type;
        size += sizeof(garble_gate) * gc->q;
        size += garble_table_size(gc) * (gc->q - gc->nxors);
        if (wires)
            size += sizeof(block) * 2 * gc->r;
        size += sizeof(int) * gc->m;
        size += sizeof(bool) * gc->m;
        size += sizeof gc->fixed_label;
        size += sizeof gc->global_key;
    }

    return size;
}

static inline size_t
cpy_to_buf(void *out, const void *in, size_t size)
{
    (void) memcpy(out, in, size);
    return size;
}

char *
garble_to_buffer(const garble_circuit *gc, char *buf, bool table_only, bool wires)
{
    size_t p = 0;
    const size_t size = garble_size(gc, table_only, wires);

    if (buf == NULL) {
        buf = calloc(1, size);
        if (buf == NULL)
            return NULL;
    }

    if (table_only) {
        p += cpy_to_buf(buf + p, &gc->q, sizeof gc->q);
        p += cpy_to_buf(buf + p, &gc->nxors, sizeof gc->nxors);
        p += cpy_to_buf(buf + p, gc->table, garble_table_size(gc) * (gc->q - gc->nxors));
        p += cpy_to_buf(buf + p, &gc->fixed_label, sizeof(block));
        p += cpy_to_buf(buf + p, &gc->global_key, sizeof(block));
        p += cpy_to_buf(buf + p, gc->output_perms, sizeof(bool) * gc->m);
    } else {
        p += cpy_to_buf(buf + p, &gc->n, sizeof gc->n);
        p += cpy_to_buf(buf + p, &gc->m, sizeof gc->m);
        p += cpy_to_buf(buf + p, &gc->q, sizeof gc->q);
        p += cpy_to_buf(buf + p, &gc->r, sizeof gc->r);
        p += cpy_to_buf(buf + p, &gc->nxors, sizeof gc->nxors);
        p += cpy_to_buf(buf + p, &gc->type, sizeof gc->type);
        p += cpy_to_buf(buf + p, gc->gates, sizeof(garble_gate) * gc->q);
        p += cpy_to_buf(buf + p, gc->table, garble_table_size(gc) * (gc->q - gc->nxors));
        if (wires)
            p += cpy_to_buf(buf + p, gc->wires, sizeof(block) * 2 * gc->r);
        p += cpy_to_buf(buf + p, gc->outputs, sizeof(int) * gc->m);
        p += cpy_to_buf(buf + p, gc->output_perms, sizeof(bool) * gc->m);
        p += cpy_to_buf(buf + p, &gc->fixed_label, sizeof(block));
        p += cpy_to_buf(buf + p, &gc->global_key, sizeof(block));
    }
    assert(p == size);
    return buf;
}

int
garble_from_buffer(garble_circuit *gc, const char *buf, bool table_only, bool wires)
{
    size_t p = 0;

    if (gc == NULL || buf == NULL)
        return GARBLE_ERR;

    memset(gc, '\0', sizeof(garble_circuit));

    if (table_only) {
        p += cpy_to_buf(&gc->q, buf + p, sizeof gc->q);
        p += cpy_to_buf(&gc->nxors, buf + p, sizeof gc->nxors);
        if ((gc->table = malloc((gc->q - gc->nxors) * garble_table_size(gc))) == NULL) {
            goto error;
        }
        p += cpy_to_buf(gc->table, buf + p, garble_table_size(gc) * (gc->q - gc->nxors));
        p += cpy_to_buf(&gc->fixed_label, buf + p, sizeof(block));
        p += cpy_to_buf(&gc->global_key, buf + p, sizeof(block));
        if ((gc->output_perms = malloc(sizeof(bool) * gc->m)) == NULL) {
            goto error;
        }
        p += cpy_to_buf(gc->output_perms, buf + p, sizeof(bool) * gc->m);
    } else {
        p += cpy_to_buf(&gc->n, buf + p, sizeof gc->n);
        p += cpy_to_buf(&gc->m, buf + p, sizeof gc->m);
        p += cpy_to_buf(&gc->q, buf + p, sizeof gc->q);
        p += cpy_to_buf(&gc->r, buf + p, sizeof gc->r);
        p += cpy_to_buf(&gc->nxors, buf + p, sizeof gc->nxors);
        p += cpy_to_buf(&gc->type, buf + p, sizeof gc->type);
        if ((gc->gates = malloc(sizeof(garble_gate) * gc->q)) == NULL) {
            goto error;
        }
        p += cpy_to_buf(gc->gates, buf + p, sizeof(garble_gate) * gc->q);
        if ((gc->table = malloc((gc->q - gc->nxors) * garble_table_size(gc))) == NULL) {
            goto error;
        }
        p += cpy_to_buf(gc->table, buf + p, garble_table_size(gc) * (gc->q - gc->nxors));
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
        p += cpy_to_buf(gc->output_perms, buf + p, sizeof(bool) * gc->m);
        p += cpy_to_buf(&gc->fixed_label, buf + p, sizeof(block));
        p += cpy_to_buf(&gc->global_key, buf + p, sizeof(block));
    }
    return GARBLE_OK;
error:
    garble_delete(gc);
    return GARBLE_ERR;
}

int
garble_save(const garble_circuit *gc, FILE *f, bool table_only, bool wires)
{
    char *buf;
    const size_t size = garble_size(gc, table_only, wires);
    size_t res;

    buf = garble_to_buffer(gc, NULL, table_only, wires);
    res = fwrite(buf, sizeof(char), size, f);
    free(buf);
    return res == size ? GARBLE_OK : GARBLE_ERR;
}

int
garble_load(garble_circuit *gc, FILE *f, bool table_only, bool wires)
{
    size_t p = 0;

    memset(gc, '\0', sizeof(garble_circuit));

    if (table_only) {
        if ((gc->table = malloc((gc->q - gc->nxors) * garble_table_size(gc))) == NULL)
            goto error;
        p += fread(gc->table, garble_table_size(gc), gc->q - gc->nxors, f);
        p += fread(&gc->fixed_label, sizeof(block), 1, f);
        p += fread(&gc->global_key, sizeof(block), 1, f);
        if ((gc->output_perms = malloc(sizeof(bool) * gc->m)) == NULL)
            goto error;
        p += fread(gc->output_perms, sizeof(bool), gc->m, f);
    } else {
        p += fread(&gc->n, sizeof gc->n, 1, f);
        p += fread(&gc->m, sizeof gc->m, 1, f);
        p += fread(&gc->q, sizeof gc->q, 1, f);
        p += fread(&gc->r, sizeof gc->r, 1, f);
        p += fread(&gc->r, sizeof gc->nxors, 1, f);
        p += fread(&gc->type, sizeof gc->type, 1, f);
        if ((gc->gates = malloc(sizeof(garble_gate) * gc->q)) == NULL)
            goto error;
        p += fread(gc->gates, sizeof(garble_gate), gc->q, f);
        if ((gc->table = malloc((gc->q - gc->nxors) * garble_table_size(gc))) == NULL)
            goto error;
        p += fread(gc->table, garble_table_size(gc), gc->q - gc->nxors, f);
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
        p += fread(gc->output_perms, sizeof(bool), gc->m, f);
        p += fread(&gc->fixed_label, sizeof(block), 1, f);
        p += fread(&gc->global_key, sizeof(block), 1, f);
    }
    return GARBLE_OK;
error:
    garble_delete(gc);
    return GARBLE_ERR;
}
