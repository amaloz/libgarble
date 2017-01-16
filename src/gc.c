#include "garble.h"

#include <assert.h>
#include <string.h>

int
garble_new(garble_circuit *gc, size_t n, size_t m, garble_type_e type)
{
    if (gc == NULL)
        return GARBLE_ERR;

    memset(gc, '\0', sizeof(garble_circuit));
    gc->outputs = calloc(m, sizeof(int));
    gc->type = type;
    gc->n = n;
    gc->m = m;
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
    memset(gc, '\0', sizeof(garble_circuit));
}

void
garble_fprint(FILE *fp, garble_circuit *gc)
{
    fprintf(fp, "n=%lu m=%lu q=%lu (nxors=%lu) r=%lu\n", gc->n, gc->m, gc->q, gc->nxors, gc->r);
    fprintf(fp, "gates=%p table=%p wires=%p outputs=%p output_perms=%p\n",
            gc->gates, gc->table, gc->wires, gc->outputs, gc->output_perms);
    block_fprintf(fp, "%B %B\n", gc->fixed_label, gc->global_key);
}

size_t
garble_size(const garble_circuit *gc, bool table_only, bool wires)
{
    size_t size = 0;
    if (gc->q < gc->nxors) {
        fprintf(stderr, "error: number of gates (%lu) < number of xor gates (%lu)\n",
                gc->q, gc->nxors);
        return 0;
    }

    size += sizeof gc->n + sizeof gc->m + sizeof gc->q + sizeof gc->r + sizeof gc->nxors;
    size += sizeof gc->type;
    size += garble_table_size(gc) * (gc->q - gc->nxors);
    size += sizeof gc->fixed_label;
    size += sizeof gc->global_key;
    size += sizeof(bool) * gc->m;

    if (!table_only) {
        size += sizeof(garble_gate) * gc->q;
        if (wires)
            size += sizeof(block) * 2 * gc->r;
        size += sizeof(int) * gc->m;
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
    if (buf == NULL) {
        const size_t size = garble_size(gc, table_only, wires);
        buf = calloc(1, size);
        if (buf == NULL)
            return NULL;
    }

    p += cpy_to_buf(buf + p, &gc->n, sizeof gc->n);
    p += cpy_to_buf(buf + p, &gc->m, sizeof gc->m);
    p += cpy_to_buf(buf + p, &gc->q, sizeof gc->q);
    p += cpy_to_buf(buf + p, &gc->r, sizeof gc->r);
    p += cpy_to_buf(buf + p, &gc->nxors, sizeof gc->nxors);
    p += cpy_to_buf(buf + p, &gc->type, sizeof gc->type);
    p += cpy_to_buf(buf + p, gc->table, garble_table_size(gc) * (gc->q - gc->nxors));    
    p += cpy_to_buf(buf + p, &gc->fixed_label, sizeof(block));
    p += cpy_to_buf(buf + p, &gc->global_key, sizeof(block));
    p += cpy_to_buf(buf + p, gc->output_perms, sizeof(bool) * gc->m);

    if (!table_only) {
        p += cpy_to_buf(buf + p, gc->gates, sizeof(garble_gate) * gc->q);
        if (wires)
            p += cpy_to_buf(buf + p, gc->wires, sizeof(block) * 2 * gc->r);
        p += cpy_to_buf(buf + p, gc->outputs, sizeof(int) * gc->m);
    }
    return buf;
}

int
garble_from_buffer(garble_circuit *gc, const char *buf, bool table_only, bool wires)
{
    size_t p = 0;

    if (gc == NULL || buf == NULL)
        return GARBLE_ERR;

    p += cpy_to_buf(&gc->n, buf + p, sizeof gc->n);
    p += cpy_to_buf(&gc->m, buf + p, sizeof gc->m);
    p += cpy_to_buf(&gc->q, buf + p, sizeof gc->q);
    p += cpy_to_buf(&gc->r, buf + p, sizeof gc->r);
    p += cpy_to_buf(&gc->nxors, buf + p, sizeof gc->nxors);
    p += cpy_to_buf(&gc->type, buf + p, sizeof gc->type);
    if ((gc->table = calloc(gc->q - gc->nxors, garble_table_size(gc))) == NULL) {
        goto error;
    }
    p += cpy_to_buf(gc->table, buf + p, garble_table_size(gc) * (gc->q - gc->nxors));
    p += cpy_to_buf(&gc->fixed_label, buf + p, sizeof(block));
    p += cpy_to_buf(&gc->global_key, buf + p, sizeof(block));
    if ((gc->output_perms = calloc(gc->m, sizeof(bool))) == NULL) {
        goto error;
    }
    p += cpy_to_buf(gc->output_perms, buf + p, sizeof(bool) * gc->m);

    if (!table_only) {
        if ((gc->gates = calloc(gc->q, sizeof(garble_gate))) == NULL) {
            goto error;
        }
        p += cpy_to_buf(gc->gates, buf + p, sizeof(garble_gate) * gc->q);
        if (wires) {
            if ((gc->wires = calloc(2 * gc->r, sizeof(block))) == NULL) {
                goto error;
            }
            p += cpy_to_buf(gc->wires, buf + p, sizeof(block) * 2 * gc->r);
        } else {
            gc->wires = NULL;
        }
        if ((gc->outputs = calloc(gc->m, sizeof(int))) == NULL) {
            goto error;
        }
        p += cpy_to_buf(gc->outputs, buf + p, sizeof(int) * gc->m);
    }
    return GARBLE_OK;
error:
    garble_delete(gc);
    return GARBLE_ERR;
}

int
garble_save(const garble_circuit *gc, FILE *f, bool table_only, bool wires)
{
    const size_t size = garble_size(gc, table_only, wires);
    char *buf;
    size_t res;

    (void) fwrite(&size, sizeof(char), sizeof size, f);
    buf = calloc(size, sizeof(char));
    if (garble_to_buffer(gc, buf, table_only, wires) == NULL) {
        fprintf(stderr, "[%s] failed to save circuit to buffer\n", __func__);
        free(buf);
        return GARBLE_ERR;
    }
    res = fwrite(buf, sizeof(char), size, f);
    free(buf);
    return res == size ? GARBLE_OK : GARBLE_ERR;
}

int
garble_load(garble_circuit *gc, FILE *f, bool table_only, bool wires)
{
    size_t size;
    char *buf;

    (void) fread(&size, sizeof(char), sizeof size, f);
    buf = calloc(size, sizeof(char));
    (void) fread(buf, sizeof(char), size, f);
    if (garble_from_buffer(gc, buf, table_only, wires) == GARBLE_ERR) {
        fprintf(stderr, "[%s] failed to load circuit from buffer\n", __func__);
        free(buf);
        return GARBLE_ERR;
    }
    free(buf);
    return GARBLE_OK;
}
