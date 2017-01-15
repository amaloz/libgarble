#include "garble.h"
#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef HAVE_MSGPACK
#  include <msgpack.h>
#endif
#include <malloc.h>
#include <sys/stat.h>

#ifdef HAVE_MSGPACK
static long
fsize(const char *filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;
    return 0;
}
#endif

int
garble_circuit_to_file(garble_circuit *gc, char *fname)
{
#ifdef HAVE_MSGPACK
    msgpack_sbuffer *buffer;
    msgpack_packer *pk;
    FILE *f;

    if ((f = fopen(fname, "wb")) == NULL) {
        perror("fopen");
        return GARBLE_ERR;
    }
    buffer = msgpack_sbuffer_new();
    pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);
    msgpack_sbuffer_clear(buffer);

    msgpack_pack_array(pk, 3 + 4 * gc->q + gc->m + gc->m);
    msgpack_pack_int(pk, gc->n);
    msgpack_pack_int(pk, gc->m);
    msgpack_pack_int(pk, gc->q);
    for (uint64_t i = 0; i < gc->q; ++i) {
        msgpack_pack_int(pk, gc->gates[i].input0);
        msgpack_pack_int(pk, gc->gates[i].input1);
        msgpack_pack_int(pk, gc->gates[i].output);
        msgpack_pack_int(pk, gc->gates[i].type);
    }
    for (uint64_t i = 0; i < gc->m; ++i) {
        msgpack_pack_int(pk, gc->outputs[i]);
    }
    for (uint64_t i = 0; i < gc->m; ++i) {
        msgpack_pack_int(pk, gc->output_perms[i]);
    }
    fwrite(buffer->data, sizeof(char), buffer->size, f);
    fclose(f);
    msgpack_packer_free(pk);
    msgpack_sbuffer_free(buffer);
    return 0;
#else
    (void) gc; (void) fname;
    (void) fprintf(stderr, "need msgpack for garble_circuit_to_file\n");
    return GARBLE_ERR;
#endif
}

int
garble_circuit_from_file(garble_circuit *gc, char *fname)
{
#ifdef HAVE_MSGPACK
    msgpack_sbuffer buffer;
    msgpack_unpacked msg;
    msgpack_object *p;
    FILE *f = NULL;
    int res = GARBLE_ERR;

    msgpack_sbuffer_init(&buffer);

    if ((buffer.size = fsize(fname)) == 0)
        goto cleanup;
    if ((f = fopen(fname, "rb")) == NULL) {
        perror("fopen");
        goto cleanup;
    }

    if ((buffer.data = malloc(buffer.size)) == NULL)
        goto cleanup;
    (void) fread(buffer.data, sizeof(char), buffer.size, f);

    msgpack_unpacked_init(&msg);
    msgpack_unpack_next(&msg, buffer.data, buffer.size, NULL);
    p = msg.data.via.array.ptr;
    gc->n = (*p++).via.i64;
    gc->m = (*p++).via.i64;
    gc->q = (*p++).via.i64;
    gc->r = gc->n + gc->q;

    gc->gates = calloc(gc->q, sizeof(garble_gate));
    gc->table = NULL;
    gc->wires = calloc(2 * gc->r, sizeof(block));
    gc->outputs = calloc(gc->m, sizeof(int));

    for (uint64_t i = 0; i < gc->q; ++i) {
        gc->gates[i].input0 = (*p++).via.i64;
        gc->gates[i].input1 = (*p++).via.i64;
        gc->gates[i].output = (*p++).via.i64;
        gc->gates[i].type = (*p++).via.i64;
    }
    for (uint64_t i = 0; i < gc->m; ++i) {
        gc->outputs[i] = (*p++).via.i64;
    }
    for (uint64_t i = 0; i < gc->m; ++i) {
        gc->output_perms[i] = (*p++).via.i64;
    }
    msgpack_unpacked_destroy(&msg);

    res = GARBLE_OK;
cleanup:
    if (f)
        fclose(f);
    msgpack_sbuffer_destroy(&buffer);
    return res;
#else
    (void) gc; (void) fname;
    (void) fprintf(stderr, "need msgpack for garble_circuit_from_file\n");
    return GARBLE_ERR;
#endif
}
